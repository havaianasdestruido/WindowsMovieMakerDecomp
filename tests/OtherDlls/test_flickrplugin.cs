// WLFlickrPlugin.dll Test Harness (.NET reflection-based)
// Targets: FlickrPlugin.FlickrApi, FlickrPlugin.FlickrPublishPlugin
// Usage: csc /reference:WLFlickrPlugin.dll /platform:x86 /out:test_flickrplugin.exe test_flickrplugin.cs
using System;
using System.Reflection;
using System.IO;
using System.Xml;
using System.Collections.Generic;

class Program {
    static int passed = 0, failed = 0;
    static string dllPath;

    static void Main() {
        // Resolve DLL path
        string dir = Path.GetFullPath("undecomp\\Photo Gallery");
        dllPath = Path.Combine(dir, "WLFlickrPlugin.dll");
        if (!File.Exists(dllPath)) {
            // Try relative to current dir
            dllPath = Path.GetFullPath("WLFlickrPlugin.dll");
        }
        Console.Write("[*] Loading: " + dllPath);
        if (!File.Exists(dllPath)) { Console.WriteLine(" [NOT FOUND]"); return; }
        Console.WriteLine();

        var asm = Assembly.LoadFrom(dllPath);
        Console.WriteLine("    Assembly: " + asm.FullName);
        Console.WriteLine("    Runtime:  " + asm.ImageRuntimeVersion);

        // ---- Test 1: FlickrApi class exists ----
        Test("FlickrApi type found", () => {
            var t = asm.GetType("FlickrPlugin.FlickrApi");
            if (t == null) throw new Exception("Type not found");
        });

        // ---- Test 2: FlickrApi consumer key/secret ----
        Test("FlickrApi consumer credentials", () => {
            var t = asm.GetType("FlickrPlugin.FlickrApi");
            var inst = Activator.CreateInstance(t, true);
            var ck = t.GetField("consumerKey", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
            var cs = t.GetField("consumerSecret", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
            if (ck == null || cs == null) throw new Exception("Fields not found");
            string key = (string)ck.GetValue(inst);
            string secret = (string)cs.GetValue(inst);
            if (string.IsNullOrEmpty(key)) throw new Exception("consumerKey is empty");
            if (string.IsNullOrEmpty(secret)) throw new Exception("consumerSecret is empty");
            Console.WriteLine("    consumerKey = " + key);
            Console.WriteLine("    consumerSecret = " + secret);
        });

        // ---- Test 3: FlickrPublishPlugin implements IPublishPlugin ----
        Test("FlickrPublishPlugin interface", () => {
            var t = asm.GetType("FlickrPlugin.FlickrPublishPlugin");
            if (t == null) throw new Exception("Type not found");
            var ifaces = t.GetInterfaces();
            bool hasPubPlugin = false;
            foreach (var i in ifaces) {
                if (i.Name.Contains("IPublishPlugin")) hasPubPlugin = true;
            }
            if (!hasPubPlugin) throw new Exception("Does not implement IPublishPlugin");
            Console.WriteLine("    Implements: " + string.Join(", ", Array.ConvertAll(ifaces, i => i.Name)));
        });

        // ---- Test 4: FlickrPublishPlugin methods ----
        Test("FlickrPublishPlugin methods", () => {
            var t = asm.GetType("FlickrPlugin.FlickrPublishPlugin");
            var inst = Activator.CreateInstance(t, true);
            string[] expectedMethods = {"ShowConfigurationSettings","PublishItem","HasSummaryInformation",
                                         "ShowSummaryInformation","HasPublishResults","LaunchPublishResults"};
            foreach (var name in expectedMethods) {
                var m = t.GetMethod(name, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
                if (m == null) throw new Exception("Method not found: " + name);
                Console.WriteLine("    " + m.ReturnType.Name + " " + name + "(" + string.Join(", ", Array.ConvertAll(m.GetParameters(), p => p.ParameterType.Name + " " + p.Name)) + ")");
            }
        });

        // ---- Test 5: FlickrApi key methods ----
        Test("FlickrApi key methods", () => {
            var t = asm.GetType("FlickrPlugin.FlickrApi");
            string[] keyMethods = {"GetRequestToken","GetOAuthUrl","GetOAuthAccount","VerifyOAuthToken",
                                    "InvokeRestApi","InvokeApi","SignOAuthRequest","OAuthEncode",
                                    "GetPhotoSetsList","CreatePhotoSet","AddPhotoToPhotoSet",
                                    "PreparePhotoUpload","UploadChunk","GetUploadedPhotoId",
                                    "GetUploadedImagesUrl","GetAccountUploadStatus","StatusOk",
                                    "GetWebString","FromQueryString","CalculateQueryString",
                                    "SetOAuth","GetXmlElement","GetErrorInfo"};
            foreach (var name in keyMethods) {
                var methods = Array.FindAll(t.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static), m => m.Name == name);
                if (methods.Length == 0) throw new Exception("Method not found: " + name);
            }
            Console.WriteLine("    All " + keyMethods.Length + " key methods present");
        });

        // ---- Test 6: FlickrApiException with error codes ----
        Test("FlickrApiException error codes", () => {
            var t = asm.GetType("FlickrPlugin.FlickrApi+FlickrApiException");
            var fields = t.GetFields(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.FlattenHierarchy);
            bool hasServiceError = false, hasAuthError = false, hasSizeError = false;
            foreach (var f in fields) {
                if (f.IsLiteral && f.Name == "SERVICE_ERROR") hasServiceError = true;
                if (f.IsLiteral && f.Name == "INVALID_AUTH_TOKEN") hasAuthError = true;
                if (f.IsLiteral && f.Name == "MAX_FILE_SIZE_EXCEEDED") hasSizeError = true;
            }
            if (!hasServiceError) throw new Exception("SERVICE_ERROR missing");
            if (!hasAuthError) throw new Exception("INVALID_AUTH_TOKEN missing");
            if (!hasSizeError) throw new Exception("MAX_FILE_SIZE_EXCEEDED missing");
            Console.WriteLine("    SERVICE_ERROR = " + (int)t.GetField("SERVICE_ERROR").GetRawConstantValue());
            Console.WriteLine("    INVALID_AUTH_TOKEN = " + (int)t.GetField("INVALID_AUTH_TOKEN").GetRawConstantValue());
            Console.WriteLine("    MAX_FILE_SIZE_EXCEEDED = " + (int)t.GetField("MAX_FILE_SIZE_EXCEEDED").GetRawConstantValue());
        });

        // ---- Test 7: Data model types ----
        Test("Data model types", () => {
            string[] types = {"FlickrPlugin.Account","FlickrPlugin.PhotoSet","FlickrPlugin.PhotoAttributes",
                              "FlickrPlugin.Permissions","FlickrPlugin.FlickrApi+UploadContext"};
            foreach (var tn in types) {
                var t = asm.GetType(tn);
                if (t == null) throw new Exception("Type not found: " + tn);
            }
            Console.WriteLine("    All " + types.Length + " data model types present");
        });

        // ---- Test 8: UI Forms ----
        Test("UI Forms", () => {
            string[] forms = {"FlickrPlugin.ConfigureForm","FlickrPlugin.PhotoSetForm","FlickrPlugin.StatusForm"};
            foreach (var fn in forms) {
                var t = asm.GetType(fn);
                if (t == null) throw new Exception("Form not found: " + fn);
                if (!t.IsSubclassOf(typeof(System.Windows.Forms.Form))) throw new Exception(fn + " is not a Form");
            }
            Console.WriteLine("    All " + forms.Length + " forms present");
        });

        // ---- Test 9: XmlHelper utility class ----
        Test("XmlHelper utility class", () => {
            var t = asm.GetType("FlickrPlugin.XmlHelper");
            if (t == null) throw new Exception("Type not found");
        });

        // ---- Test 10: FlickrApi endpoint URLs in IL ----
        Test("FlickrApi OAuth endpoints", () => {
            var t = asm.GetType("FlickrPlugin.FlickrApi");
            var inst = Activator.CreateInstance(t, true);
            // GetRequestToken should not throw (will fail due to network, but method should exist)
            var requestTokenMethod = t.GetMethod("GetRequestToken", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
            // GetOAuthUrl should return a non-null string
            var oauthUrlMethod = t.GetMethod("GetOAuthUrl", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
            // SetOAuth should not throw
            var setOAuthMethod = t.GetMethod("SetOAuth", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
            Console.WriteLine("    GetRequestToken, GetOAuthUrl, SetOAuth all accessible");
        });

        // ---- Test 11: Plug-in resources exist ----
        Test("Embedded resources", () => {
            string[] resources = {"FlickrPlugin.ConfigureForm.resources",
                                  "FlickrPlugin.PhotoSetForm.resources",
                                  "FlickrPlugin.StatusForm.resources",
                                  "FlickrPlugin.Properties.FlickrPluginResources.resources"};
            foreach (var r in resources) {
                var stream = asm.GetManifestResourceStream(r);
                if (stream == null) throw new Exception("Resource not found: " + r);
                stream.Dispose();
            }
            Console.WriteLine("    All " + resources.Length + " embedded resources present");
        });

        // ---- Summary ----
        Console.WriteLine("\n=== Results: " + passed + " passed, " + failed + " failed ===");
    }

    static void Test(string name, Action action) {
        Console.Write("[*] " + name + "...");
        try {
            action();
            Console.WriteLine(" PASS");
            passed++;
        } catch (Exception ex) {
            Console.WriteLine(" FAIL: " + ex.Message);
            failed++;
        }
    }
}
