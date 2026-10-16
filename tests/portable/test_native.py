"""Compile production algorithms with inert platform shims, never copies of their logic.

Requires g++ (C++14), ASan/UBSan. These are not substitutes for Windows DLL tests.
WMMR_TEST_SOURCE_ROOT can point at an older checkout to demonstrate regressions.
"""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(os.environ.get('WMMR_TEST_SOURCE_ROOT', Path(__file__).resolve().parents[2]))


def source(path):
    return (ROOT / path).read_text(encoding='utf-8-sig')


def body(text, start, end):
    return text[text.index(start):text.index(end, text.index(start))]


def no_includes(text):
    return '\n'.join(line for line in text.splitlines() if not line.startswith(('#include', '#pragma once'))) + '\n'


COMMON = r'''
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <cassert>
#include <memory>
#include <limits>
#include <cctype>
using BYTE=unsigned char; using DWORD=uint32_t; using UINT=uint32_t;
using UINT16=uint16_t; using UINT32=uint32_t; using UINT64=uint64_t;
using HRESULT=int32_t; using BOOL=int; using WCHAR=wchar_t; using LONGLONG=int64_t;
constexpr HRESULT S_OK=0, S_FALSE=1, E_FAIL=-1, E_INVALIDARG=-2, E_POINTER=-3,
 E_ABORT=-4, E_OUTOFMEMORY=-5, E_UNEXPECTED=-6;
#define FAILED(hr) ((hr)<0)
#define SUCCEEDED(hr) ((hr)>=0)
#define TRUE 1
#define FALSE 0
#define UNREFERENCED_PARAMETER(x) (void)(x)
#define ZeroMemory(p,n) std::memset(p,0,n)
#define _TRUNCATE 0
inline void wcsncpy_s(wchar_t* p,size_t n,const wchar_t* s,int) {std::wcsncpy(p,s,n-1);p[n-1]=0;}
'''


class NativeRegressions(unittest.TestCase):
    def compile_run(self, code):
        with tempfile.TemporaryDirectory() as tmp:
            cpp = Path(tmp) / 'regression.cpp'
            exe = Path(tmp) / 'regression'
            cpp.write_text(COMMON + code)
            compile = subprocess.run(['g++', '-std=c++14', '-g', '-O1', '-pthread',
                                      '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                                      '-fno-pie', '-no-pie', '-I', str(ROOT), str(cpp), '-o', str(exe)],
                                     capture_output=True, text=True)
            self.assertEqual(compile.returncode, 0, compile.stderr)
            result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30,
                                    env={**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1'})
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def mp4(self):
        header = source('src/WLXMP4Parser/WLXMP4Parser.h')
        info = body(header, '    struct MP4TrackInfo', '    struct MP4ChapterInfo')
        cpp = source('src/WLXMP4Parser/WLXMP4Parser.cpp')
        algorithms = body(cpp, '// Big-endian read helpers', '// MP4File -- main parser object')
        # Publicize private parser entry points only in the harness.
        return info + algorithms.replace('private:', 'public:')

    def test_mp4_box_overflow_and_exact_remaining_15(self):
        self.compile_run(self.mp4() + r'''
int main() {
    const BYTE* payload=nullptr; UINT64 size=0;
    std::vector<BYTE> bytes(24, 0xff);
    BYTE prefix[]={0,0,0,8,'f','r','e','e', 0,0,0,1,'m','o','o','v'};
    std::memcpy(bytes.data(),prefix,16);
    BoxReader bad(bytes.data(),bytes.size());
    assert(bad.Next(nullptr,&payload,&size) && size==0);
    assert(!bad.Next(nullptr,&payload,&size));
    for(size_t n=0;n<16;++n) {
        std::vector<BYTE> truncated(n,0);
        if(n>=8) truncated[3]=1;
        BoxReader reader(truncated.data(),n);
        assert(!reader.Next(nullptr,&payload,&size));
    }
    BYTE exact[]={0,0,0,8,'f','r','e','e',0xaa};
    BoxReader reader(exact,9);
    assert(reader.Next(nullptr,&payload,&size) && size==0);
    assert(!reader.Next(nullptr,&payload,&size));
    exact[3]=0;
    BoxReader rest(exact,9);
    assert(rest.Next(nullptr,&payload,&size) && size==1);
}
''')

    def test_mp4_file_walker_overflow_15(self):
        cpp = body(source('src/WLXMP4Parser/WLXMP4Parser.cpp'), 'class MP4File\n{', '} // namespace MP4Parser')
        shim = r'''
using LPCWSTR=const wchar_t*; using HANDLE=void*;
struct LARGE_INTEGER {LONGLONG QuadPart;};
constexpr HANDLE INVALID_HANDLE_VALUE=nullptr;
constexpr int GENERIC_READ=1,FILE_SHARE_READ=1,OPEN_EXISTING=1,FILE_ATTRIBUTE_NORMAL=1,FILE_BEGIN=0;
#define HRESULT_FROM_WIN32(x) (-HRESULT(x))
static std::vector<BYTE> fileBytes; static size_t filePos=0;
HANDLE CreateFileW(LPCWSTR,int,int,void*,int,int,void*){filePos=0;return reinterpret_cast<void*>(1);}
DWORD GetLastError(){return 1;}
BOOL GetFileSizeEx(HANDLE,LARGE_INTEGER* size){size->QuadPart=fileBytes.size();return TRUE;}
BOOL SetFilePointerEx(HANDLE,LARGE_INTEGER pos,void*,int){assert(pos.QuadPart>=0);filePos=pos.QuadPart;return TRUE;}
BOOL ReadFile(HANDLE,BYTE* out,DWORD n,DWORD* read,void*){
 assert(filePos<=fileBytes.size());*read=std::min<size_t>(n,fileBytes.size()-filePos);
 if(*read)std::memcpy(out,fileBytes.data()+filePos,*read);filePos+=*read;return TRUE;
}
void CloseHandle(HANDLE){}
'''
        self.compile_run(self.mp4() + shim + cpp + r'''
int main(){
 MP4File file;
 fileBytes={0,0,0,8,'f','r','e','e',0,0,0,1,'m','o','o','v'};
 fileBytes.resize(24,0xff);
 assert(FAILED(file.Open(L"overflow.mp4")));
 UINT32 count=999;assert(file.GetTrackCount(&count)==E_UNEXPECTED);
 fileBytes={0,0,0,16,'m','o','o','v',0,0,0,8,'f','r','e','e'};
 assert(file.Open(L"valid.mp4")==S_OK);
 assert(file.GetTrackCount(&count)==S_OK && count==0);
}
''')

    def test_mp4_field_boundaries_16(self):
        self.compile_run(self.mp4() + r'''
int main() {
    for (unsigned version=0; version<=1; ++version) {
        for(size_t n=0;n<=100;++n) {
            std::vector<BYTE> bytes(n,0);
            if(n) bytes[0]=version;
            TrakBox trak; MoovBox moov; StblBox stbl;
            trak.ParseTkhd(bytes.data(),n);
            trak.ParseMdhd(bytes.data(),n);
            trak.ParseHdlr(bytes.data(),n);
            moov.ParseMvhd(bytes.data(),n);
            if(n>=8) bytes[7]=1;
            stbl.ParseStsd(bytes.data(),n,TRUE);
            stbl.ParseStts(bytes.data(),n);
            stbl.ParseStsz(bytes.data(),n);
        }
    }
    std::vector<BYTE> tkhd(84,0);
    tkhd[15]=7; tkhd[23]=9; tkhd[77]=2; tkhd[81]=1;
    TrakBox track; track.ParseTkhd(tkhd.data(),tkhd.size());
    assert(track.GetTrackInfo().uTrackId==7);
    assert(track.GetTrackInfo().uDuration==9);
    assert(track.GetTrackInfo().uWidth==2 && track.GetTrackInfo().uHeight==1);
}
''')

    def test_undo_redo_failure_retry_and_order_20(self):
        header = no_includes(source('src/MovieMakerCore/SundanceApp/UndoManager.h'))
        cpp = no_includes(source('src/MovieMakerCore/SundanceApp/UndoManager.cpp'))
        self.compile_run(header + cpp + r'''
int main() {
    UndoManager manager; bool failUndo=true,failRedo=true; int state=1;
    manager.Push([&] {if(failRedo) return E_FAIL; ++state; return S_OK;},
                 [&] {if(failUndo) return E_FAIL; --state; return S_OK;});
    assert(manager.Undo()==E_FAIL && manager.GetUndoCount()==1 && !manager.CanRedo());
    failUndo=false;
    assert(manager.Undo()==S_OK && state==0 && manager.GetRedoCount()==1);
    assert(manager.Redo()==E_FAIL && manager.GetRedoCount()==1 && !manager.CanUndo());
    failRedo=false;
    assert(manager.Redo()==S_OK && state==1 && manager.GetUndoCount()==1);
    manager.Push([]{return S_OK;},[]{return S_OK;});
    assert(manager.Undo()==S_OK && manager.Undo()==S_OK);
    assert(manager.Undo()==S_FALSE);
    assert(manager.Redo()==S_OK && manager.Redo()==S_OK && manager.Redo()==S_FALSE);
}
''')

    def test_reload_restart_cancel_destroy_18(self):
        header = body(source('src/MovieMakerCore/HMREngine/AsyncWorker.h'), '    class ReloadImage', '    // --- EnumerateNodes')
        cpp = body(source('src/MovieMakerCore/HMREngine/AsyncWorker.cpp'), 'ReloadImage::ReloadImage()', '// EnumerateNodes')
        shim = r'''
struct ID3D11Device {};
struct ID3D11Texture2D {void AddRef(){}};
struct ID3D11ShaderResourceView {void AddRef(){}};
template<class T> struct CComPtr {T* value=nullptr; operator T*()const{return value;}};
'''
        self.compile_run(shim + header + cpp + r'''
int main() {
    ID3D11Device device;
    for(int i=0;i<100;++i) {
        std::atomic<int> calls{0};
        ReloadImage worker;
        worker.SetCompleteCallback([&](const std::wstring&,HRESULT){++calls;});
        assert(worker.StartReload(L"first",&device)==S_OK);
        assert(worker.StartReload(L"second",&device)==S_OK);
        worker.Cancel();
    }
    ReloadImage worker;
    assert(worker.StartReload(L"",&device)==E_INVALIDARG);
}
''')

    def test_audio_queue_shrink_grow_wrap_fifo_19(self):
        header = no_includes(source('src/MovieMakerCore/HMRAVSource/Audio/AudioQueue.h')).replace('AVSOURCE_API ', '')
        cpp = no_includes(source('src/MovieMakerCore/HMRAVSource/Audio/AudioQueue.cpp'))
        cpp = cpp.replace(body(cpp, 'HRESULT AudioQueue::EnqueueSample', 'HRESULT AudioQueue::Dequeue'), '')
        shim = r'''
struct IMFSample; using HANDLE=void*; using CRITICAL_SECTION=int;
constexpr DWORD INFINITE=0xffffffff,WAIT_OBJECT_0=0,ERROR_NO_DATA=1,ERROR_TIMEOUT=2;
#define HRESULT_FROM_WIN32(x) (-HRESULT(x))
inline void InitializeCriticalSection(int*){} inline void DeleteCriticalSection(int*){}
inline void EnterCriticalSection(int*){} inline void LeaveCriticalSection(int*){}
inline HANDLE CreateEvent(void*,bool,bool,void*) {return reinterpret_cast<void*>(1);}
inline void CloseHandle(HANDLE){} inline void ResetEvent(HANDLE){} inline void SetEvent(HANDLE){}
inline DWORD WaitForMultipleObjects(int,HANDLE*,bool,DWORD){return 0;}
namespace ATL {template<class T> class CAtlArray {
    std::vector<T> data;
public:
    void SetCount(size_t n){data.resize(n);} void RemoveAll(){data.clear();}
    T& GetAt(size_t n){return data.at(n);} const T& GetAt(size_t n)const{return data.at(n);}
    T* GetData(){return data.data();} const T* GetData()const{return data.data();}
};}
'''
        self.compile_run(shim + header + cpp + r'''
using namespace HMRAVSource;
int main() {
    AudioQueue queue; assert(queue.Initialize(2)==S_OK);
    BYTE value=1; queue.Enqueue(&value,1,1); ++value; queue.Enqueue(&value,1,2);
    assert(queue.SetMaxItems(1)==S_OK);
    ++value; assert(queue.Enqueue(&value,1,3)==S_OK);
    AudioQueueItem item; assert(queue.Dequeue(&item)==S_OK && item.arrData.GetAt(0)==3);
    queue.SetMaxItems(4);
    for(int i=0;i<30;++i){value=i;queue.Enqueue(&value,1,i);}
    queue.SetMaxItems(2); queue.SetMaxItems(5);
    assert(queue.Dequeue(&item)==S_OK && item.llTimestampHns==28);
    assert(queue.Dequeue(&item)==S_OK && item.llTimestampHns==29);
    assert(queue.IsEmpty()); queue.Flush();
    assert(queue.SetMaxItems(0)==E_INVALIDARG);
}
''')

    def test_frame_copy_short_padded_inverted_and_overflow_24(self):
        self.compile_run(r'''
#include "src/Playback/FrameCopy.h"
int main() {
    BYTE input[24]; for(int i=0;i<24;++i)input[i]=i+1;
    BYTE output[32]={};
    for(size_t size=0;size<20;++size)
        assert(!DirectUI::CopyRGB32Rows(output,16,input,size,2,2,12));
    assert(DirectUI::CopyRGB32Rows(output,16,input,20,2,2,12));
    assert(output[0]==1 && output[16]==13 && output[8]==0);
    assert(DirectUI::CopyRGB32Rows(output,16,input,20,2,2,-12));
    assert(output[0]==13 && output[16]==1);
    assert(!DirectUI::CopyRGB32Rows(output,16,input,24,SIZE_MAX,2,12));
    assert(!DirectUI::CopyRGB32Rows(output,16,input,24,2,SIZE_MAX,12));
    assert(!DirectUI::CopyRGB32Rows(output,16,input,24,2,2,INT32_MIN));
    assert(!DirectUI::CopyRGB32Rows(output,4,input,24,2,2,12));
}
''')

    def test_playback_null_sample_cap_and_copy_failure_24(self):
        cpp = body(source('src/Playback/PlaybackEngine.cpp'), 'HRESULT PlaybackEngine::ReadNextSample()', 'HRESULT PlaybackEngine::CopySampleToTexture')
        self.compile_run(r'''
constexpr HRESULT MF_E_NOT_INITIALIZED=-10,MF_E_INVALIDMEDIATYPE=-11;
constexpr DWORD MF_SOURCE_READER_FIRST_VIDEO_STREAM=0,MF_SOURCE_READERF_ENDOFSTREAM=1,
 MF_SOURCE_READERF_ERROR=2,MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED=4;
struct IMFSample {int refs=0;void Release(){--refs;}};
template<class T>void SafeReleasePlayback(T*& p){if(p){p->Release();p=nullptr;}}
struct Reader {
 int calls=0, mode=0;IMFSample sample;
 HRESULT ReadSample(DWORD,DWORD,DWORD*,DWORD* flags,LONGLONG* time,IMFSample** out){
   ++calls;*flags=0;*time=0;*out=nullptr;
   if(mode==1)*flags=MF_SOURCE_READERF_ENDOFSTREAM;
   if(mode==2){*out=&sample;++sample.refs;}
   if(mode==3){*out=&sample;++sample.refs;return E_FAIL;}
   if(mode==4)*flags=MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED;
   return S_OK;
 }
};
struct PlaybackEngine {
 Reader* m_pReader=nullptr;bool m_eof=false;double m_currentTime=0;
 HRESULT ReadNextSample();HRESULT CopySampleToTexture(IMFSample*){return E_FAIL;}
};
''' + cpp + r'''
int main(){
 Reader reader;PlaybackEngine engine;engine.m_pReader=&reader;
 assert(engine.ReadNextSample()==E_FAIL && reader.calls==64);
 reader.mode=1;assert(engine.ReadNextSample()==S_FALSE && engine.m_eof);
 reader.mode=2;assert(engine.ReadNextSample()==E_FAIL && reader.sample.refs==0);
 reader.mode=3;assert(engine.ReadNextSample()==E_FAIL && reader.sample.refs==0);
 reader.mode=4;assert(engine.ReadNextSample()==MF_E_INVALIDMEDIATYPE);
}
''')

    def test_json_truncation_arrays_quotes_and_depth_17(self):
        cpp = body(source('src/MovieMakerCore/HMREngine/X3DReader.cpp'), '    HRESULT X3DReader::ParseJSON', '    bool X3DReader::ParseNodeLine')
        shim = r'''
struct Vec3{}; struct Rotation4f{}; struct Rgb{};
enum class FieldType { SFFloat,SFDouble,SFInt32,SFBool,SFVec3f,SFRotation,SFColor,SFString };
struct X3DFieldNode {FieldType GetFieldType(){return FieldType::SFFloat;} void Release(){}};
struct SFFloat:X3DFieldNode{float m_value;}; struct SFInt32:X3DFieldNode{int m_value;};
struct SFBool:X3DFieldNode{bool m_value;}; struct SFVec3f:X3DFieldNode{Vec3 m_value;};
struct SFRotation:X3DFieldNode{Rotation4f m_value;}; struct SFColor:X3DFieldNode{Rgb m_value;};
struct SFString:X3DFieldNode{std::string m_value;};
struct X3DChildNode {void AddChild(X3DChildNode*){} void SetField(const std::string&,X3DFieldNode*){}};
struct X3DParseContext {
 static constexpr size_t kMaxNestingDepth=512;
 X3DChildNode* GetCurrentNode(){return nullptr;} void PushNode(X3DChildNode*){} void PopNode(){}
};
struct X3DReader {
 HRESULT ParseJSON(const char*,size_t,X3DParseContext&);
 X3DChildNode* CreateNode(const std::string&){return nullptr;}
 X3DFieldNode* CreateField(const std::string&){return nullptr;}
 template<class T> bool ParseFloat(const std::string&,T&){return false;}
 template<class T> bool ParseInt(const std::string&,T&){return false;}
 template<class T> bool ParseBool(const std::string&,T&){return false;}
 template<class T> bool ParseVec3f(const std::string&,T&){return false;}
 template<class T> bool ParseRotation(const std::string&,T&){return false;}
 template<class T> bool ParseColor(const std::string&,T&){return false;}
};
'''
        self.compile_run(shim + cpp + r'''
int main() {
 X3DReader reader; X3DParseContext ctx;
 std::vector<std::string> invalid={"{\"a\":", "{\"a\":1", "{\"a\":[1,", "{ ", "{\"a\":\"x", "{\"a\":{", "{\"a\":1,}", "{\"a\":[1,]}", "[1,"};
 for(const auto& s:invalid) {
   std::unique_ptr<char[]> bytes(new char[s.size()]);std::memcpy(bytes.get(),s.data(),s.size());
   assert(FAILED(reader.ParseJSON(bytes.get(),s.size(),ctx)));
 }
 std::string valid="{\"a\":[1, {\"b\":\"quoted \\\" ]\"}], \"c\":{}}";
 assert(reader.ParseJSON(valid.data(),valid.size(),ctx)==S_OK);
 for(size_t n=1;n<valid.size();++n) {
   std::unique_ptr<char[]> bytes(new char[n]);std::memcpy(bytes.get(),valid.data(),n);
   assert(FAILED(reader.ParseJSON(bytes.get(),n,ctx)));
 }
 std::string deep="{\"a\":"+std::string(600,'[')+"0"+std::string(600,']')+"}";
 assert(FAILED(reader.ParseJSON(deep.data(),deep.size(),ctx)));
}
''')

    def test_autosave_worker_join_order_and_failed_wait_23(self):
        cpp = source('src/MovieMakerCore/SundanceApp/AutoSaveManager.cpp')
        methods = (body(cpp, 'void AutoSaveManager::Shutdown()', '// OnProjectOpened') +
                   body(cpp, 'void AutoSaveManager::OnProjectClosed()', '// OnProjectSaved') +
                   cpp[cpp.index('void AutoSaveManager::AutoSaveLoop()'):])
        self.compile_run(r'''
using HANDLE=int; constexpr DWORD INFINITE=0xffffffff, WAIT_TIMEOUT=258, WAIT_OBJECT_0=0;
static bool stopped=false, joined=false; static int waits=0;
void SetEvent(HANDLE){stopped=true;}
DWORD WaitForSingleObject(HANDLE h,DWORD timeout) {
    if(h==1){assert(stopped && timeout==INFINITE);joined=true;return WAIT_OBJECT_0;}
    ++waits;return 0xffffffff; // WAIT_FAILED must stop the worker, not spin.
}
void CloseHandle(HANDLE h){assert(joined);if(h==2)assert(stopped);}
void ResetEvent(HANDLE){assert(joined);stopped=false;}
struct AutoSaveManager {
 HANDLE m_hStopEvent=2,m_hTimerThread=1; void* m_pAppMain=nullptr;
 bool m_bEnabled=true,m_bSaving=false; DWORD m_dwIntervalMs=30;
 void Shutdown();void OnProjectClosed();void AutoSaveLoop();
 void DeleteRecoveryFile(){assert(joined);} void SaveNow(){assert(false);}
};
''' + methods + r'''
int main(){
 AutoSaveManager manager;manager.OnProjectClosed();assert(manager.m_hTimerThread==0);
 manager.m_hTimerThread=1;stopped=joined=false;manager.Shutdown();
 assert(manager.m_hStopEvent==0 && manager.m_hTimerThread==0);
 manager.AutoSaveLoop();assert(waits==1);
}
''')

    def test_recover_edit_save_retains_original_destination_22(self):
        cpp = source('src/MovieMakerCore/StoryboardManager/MovieProject.cpp')
        recovery = body(cpp, 'HRESULT MovieProject::RecoverAutoSave()', 'void MovieProject::DeleteAutoSaveFile()')
        self.compile_run(r'''
namespace ATL {struct CString:std::wstring {
 using std::wstring::wstring; CString(const std::wstring& s):std::wstring(s){}
};}
const wchar_t* kAutoSaveExtension=L".autosave";
constexpr DWORD ProjectDirtyFlagAll=0xffffffff;
struct MovieProject {
 ATL::CString m_strProjectPath=L"movie.wlmp"; bool hasAutoSave=true,fail=false;
 struct State{bool dirty=false;void SetDirty(DWORD){dirty=true;}}m_state;
 int content=1, originalContent=1, recoveryContent=2;
 bool HasAutoSaveFile(){return hasAutoSave;}
 HRESULT Load(const ATL::CString& path){
   m_strProjectPath=path;if(fail)return E_FAIL;content=recoveryContent;return S_OK;
 }
 HRESULT RecoverAutoSave();
 HRESULT Save(){assert(m_strProjectPath==L"movie.wlmp");originalContent=content;hasAutoSave=false;return S_OK;}
};
''' + recovery + r'''
int main(){
 MovieProject project;assert(project.RecoverAutoSave()==S_OK);
 assert(project.m_strProjectPath==L"movie.wlmp" && project.content==2 && project.m_state.dirty);
 project.content=3;assert(project.Save()==S_OK && project.originalContent==3 && !project.hasAutoSave);
 assert(project.RecoverAutoSave()==S_FALSE);
 project.hasAutoSave=true;project.fail=true;assert(project.RecoverAutoSave()==E_FAIL);
 assert(project.m_strProjectPath==L"movie.wlmp");
}
''')


if __name__ == '__main__':
    unittest.main()
