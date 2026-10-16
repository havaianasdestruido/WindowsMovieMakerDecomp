"""Structural and local-Git integration checks for #21, #25, #30, #31, #32."""
from pathlib import Path
import os
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ConfigurationRegressions(unittest.TestCase):
    def test_unused_project_model_removed_21(self):
        self.assertFalse((ROOT / 'src/Project/Project.h').exists())
        self.assertNotIn('add_subdirectory(src/Project)', (ROOT / 'CMakeLists.txt').read_text())
        self.assertNotIn('- ProjectManager', (ROOT / '.github/workflows/build-projects.yml').read_text())

    def test_core_does_not_construct_or_link_dormant_preview_25(self):
        source = (ROOT / 'src/MovieMakerCore/SundanceApp/SundanceAppMain.cpp').read_text()
        cmake = (ROOT / 'src/MovieMakerCore/CMakeLists.txt').read_text()
        for name in ('GPURenderer', 'PlaybackEngine', 'TimelineEngine', 'TimelineDispatcher'):
            self.assertNotRegex(source, r'new\s*(?:\([^)]*\))?\s*(?:DirectUI::)?' + name)
            self.assertNotRegex(cmake, r'(?m)^\s+' + name + r'\s*$')
        self.assertNotIn('m_pGPURenderer->Initialize', source)
        self.assertIn('PlaybackController', (ROOT / 'TODO_AUDIT.md').read_text())

    def test_automatic_windows_build_events_30(self):
        workflow = (ROOT / '.github/workflows/build-projects.yml').read_text()
        self.assertIn('on:\n  push:\n    branches:\n      - main\n  pull_request:\n  workflow_dispatch:', workflow)
        self.assertIn('runs-on: windows-2022', workflow)
        self.assertIn('-A Win32', workflow)
        self.assertEqual(len(re.findall(r'^          - ', workflow, re.M)), 28)

    def test_mirror_ephemeral_credentials_and_scoped_push_31(self):
        workflow = (ROOT / '.github/workflows/sync.yml').read_text()
        self.assertNotIn('--mirror', workflow)
        self.assertNotIn('git remote add', workflow)
        self.assertNotIn('https://${{', workflow)
        self.assertIn('persist-credentials: false', workflow)
        script = workflow.split('        run: |\n', 1)[1].split('        env:', 1)[0]
        script = '\n'.join(line[10:] for line in script.splitlines())
        helper = re.search(r"-c 'credential.helper=([^']+)'", script).group(1)
        with tempfile.TemporaryDirectory() as tmp:
            work = Path(tmp) / 'work'
            remote = Path(tmp) / 'mirror.git'
            def git(*args, **kwargs):
                return subprocess.run(['git', *args], cwd=work, check=True, capture_output=True, **kwargs)
            work.mkdir()
            git('init', '-b', 'main')
            git('config', 'user.email', 'regression@example.invalid')
            git('config', 'user.name', 'Regression')
            git('commit', '--allow-empty', '-m', 'fixture')
            git('tag', 'v1')
            subprocess.run(['git', 'init', '--bare', str(remote)], check=True, capture_output=True)
            git('push', str(remote), 'main:refs/heads/unrelated')
            original = (work / '.git/config').read_bytes()
            secret = 'dummy-secret-for-issue-31-test'
            env = {**os.environ, 'CODEBERG_PAT': secret, 'GIT_TERMINAL_PROMPT': '0'}
            result = git('-c', 'credential.helper=', '-c', 'credential.helper=' + helper,
                         'credential', 'fill', input=b'protocol=https\nhost=codeberg.org\n\n', env=env)
            self.assertIn(('password=' + secret).encode(), result.stdout)
            local_script = script.replace('https://codeberg.org/PatoFlamejanteTV/WindowsMovieMakerDecomp.git', str(remote))
            subprocess.run(['bash', '-e', '-c', local_script], cwd=work, env=env, check=True, capture_output=True)
            self.assertEqual(original, (work / '.git/config').read_bytes())
            for file in work.rglob('*'):
                if file.is_file():
                    self.assertNotIn(secret.encode(), file.read_bytes())
            refs = git('--git-dir=' + str(remote), 'for-each-ref', '--format=%(refname)').stdout.decode().splitlines()
            self.assertEqual(set(refs), {'refs/heads/main', 'refs/heads/unrelated', 'refs/tags/v1'})

    def test_docs_keep_strict_links_and_audit_waiver_32(self):
        self.assertIn("onBrokenLinks: 'throw'", (ROOT / 'website/docusaurus.config.js').read_text())
        readme = (ROOT / 'website/README.md').read_text()
        self.assertIn('GHSA-vfj7-8cjw-p6xm', readme)
        self.assertIn('2026-11-16', readme)


if __name__ == '__main__':
    unittest.main()
