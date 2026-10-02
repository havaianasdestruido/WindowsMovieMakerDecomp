import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import Heading from '@theme/Heading';
import CodeBlock from '@theme/CodeBlock';

import styles from './index.module.css';

const STATS = [
  {value: '29', label: 'CMake build targets'},
  {value: '375+', label: 'Reconstructed source files'},
  {value: '1360', label: 'RTTI classes mapped'},
  {value: '116', label: 'Contract tests green'},
  {value: '23', label: 'Original bugs preserved'},
  {value: '65', label: 'Analysis modules'},
];

const FEATURES = [
  {
    icon: '🧬',
    title: 'Faithful reconstruction',
    body: 'Windows Live Movie Maker 2012 (codename Sundance, 16.4.3528.0331) rebuilt from binary analysis as C++14 / ATL / WTL source, matching the original MSVC 11.0 binaries.',
  },
  {
    icon: '🧾',
    title: 'Export parity, enforced',
    body: 'Every DLL exports the same names, ordinals, and decorations as the reference binary. tools/diff_exports.py gates the export surface on every change.',
  },
  {
    icon: '🔬',
    title: 'Contract-tested behavior',
    body: 'A 116-check ctypes contract suite (tests/mmr-python) pins HRESULTs, GUIDs, factory semantics, and file-format details across the DLL surfaces.',
  },
  {
    icon: '🧱',
    title: 'Disciplined stub design',
    body: 'Telemetry is inert-but-safe, parsers are bounds-checked, credentials are DPAPI-encrypted. Every stub category is documented and intentional.',
  },
  {
    icon: '🐞',
    title: 'Quirks kept on purpose',
    body: '23 original bugs and anomalies — from the Sundance single-instance mutex to the null CLSID — are faithfully reproduced and cataloged in QUIRKS.md.',
  },
  {
    icon: '🚀',
    title: 'It launches',
    body: 'MovieMaker.exe boots, creates the "Windows Live Movie Maker" window, and runs its DirectUI/Ribbon UI pipeline on modern Windows with a VS2022 toolchain.',
  },
];

const FAMILIES = [
  {
    to: '/docs/modules/application',
    count: '4 targets',
    title: 'Application',
    body: 'MovieMaker.exe launcher, the MovieMakerCore.dll engine, localization resources, and the preview client.',
  },
  {
    to: '/docs/modules/dui-engine',
    count: '7 targets',
    title: 'DirectUI engine layer',
    body: 'UXCore, uxctl, MediaCatalog, ProjectManager, TimelineEngine, GPURenderer, and PlaybackEngine.',
  },
  {
    to: '/docs/modules/media-foundation',
    count: '5 targets',
    title: 'Media Foundation',
    body: 'WLMFDS, WLMFReadWrite, WLXMP4Parser, WLXCodecHost, and the WLXTranscode helper executable.',
  },
  {
    to: '/docs/modules/pipeline-effects',
    count: '6 targets',
    title: 'Pipeline & effects',
    body: 'WLXPipeline, WLXPipetran, WLXSlideshow, WLXPhotoCinematic, WLXVideoTrim, and WLXMovieLibrary.',
  },
  {
    to: '/docs/modules/platform',
    count: '7 targets',
    title: 'Platform services',
    body: 'WLXPhotoBase, publishing, face recognition, metadata, identity, and the inert telemetry pair.',
  },
];

const SAMPLE_BUILD = `# Configure + build (Win32 / VS2022)
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug

# Run the reconstructed app
build\\bin\\Debug\\MovieMaker.exe

# Contract suite (behavioral regression oracle)
$env:WMMR_DLL_DIR = "build\\bin\\Debug"
python32\\python.exe tests\\mmr-python\\run_tests.py   # PASS=116 FAIL=0`;

function HomepageHeader() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <header className="wmmr-hero">
      <div className="container">
        <span className="wmmr-hero--badge">
          🎞️ Windows Live Movie Maker 2012 · 16.4.3528.0331 · codename &ldquo;Sundance&rdquo;
        </span>
        <Heading as="h1" className="wmmr-hero--title">
          The complete <em>Windows Live Movie Maker</em> source reconstruction, fully documented
        </Heading>
        <p className="wmmr-hero--tagline">
          {siteConfig.tagline}
        </p>
        <div className="wmmr-hero--buttons">
          <Link
            className="button button--primary button--lg"
            to="/docs/intro">
            Get Started&nbsp;→
          </Link>
          <Link
            className="button button--secondary button--outline button--lg"
            to="/docs/modules/overview">
            Browse the Modules
          </Link>
          <a
            className="button button--secondary button--outline button--lg"
            href="https://github.com/havaianasdestruido/WindowsMovieMakerDecomp"
            target="_blank"
            rel="noreferrer noopener">
            GitHub
          </a>
        </div>
      </div>
    </header>
  );
}

function Stats() {
  return (
    <section className="wmmr-stats">
      <div className="container">
        <ul className="wmmr-stats--grid">
          {STATS.map((stat) => (
            <li key={stat.label}>
              <span className="wmmr-stats--value">{stat.value}</span>
              <span className="wmmr-stats--label">{stat.label}</span>
            </li>
          ))}
        </ul>
      </div>
    </section>
  );
}

function Features() {
  return (
    <section className="wmmr-section">
      <Heading as="h2" className="wmmr-section--title">
        Why this project is different
      </Heading>
      <p className="wmmr-section--subtitle">
        A decompilation measured not by lines of code, but by verified behavioral parity.
      </p>
      <div className="wmmr-features">
        {FEATURES.map((feature) => (
          <div key={feature.title} className="wmmr-feature">
            <div className="wmmr-feature--icon" aria-hidden="true">
              {feature.icon}
            </div>
            <div className="wmmr-feature--title">{feature.title}</div>
            <div className="wmmr-feature--body">{feature.body}</div>
          </div>
        ))}
      </div>
    </section>
  );
}

function Families() {
  return (
    <section className="wmmr-section" style={{paddingTop: 0}}>
      <Heading as="h2" className="wmmr-section--title">
        The codebase, by module family
      </Heading>
      <p className="wmmr-section--subtitle">
        29 build targets organized into five families — from the launcher EXE down to the inert
        telemetry pair. Each module has its own documentation page.
      </p>
      <div className="wmmr-families">
        {FAMILIES.map((family) => (
          <Link key={family.to} to={family.to} className="wmmr-family">
            <span className="wmmr-family--count">{family.count}</span>
            <div className="wmmr-family--title">{family.title}</div>
            <div className="wmmr-family--body">{family.body}</div>
          </Link>
        ))}
      </div>
    </section>
  );
}

function BuildSample() {
  return (
    <section className="wmmr-section" style={{paddingTop: 0}}>
      <Heading as="h2" className="wmmr-section--title">
        Build it yourself
      </Heading>
      <p className="wmmr-section--subtitle">
        MSVC-only, Win32 (x86) only — exactly like the original 2012 binaries.
      </p>
      <div className="wmmr-homepage-code">
        <CodeBlock language="powershell">{SAMPLE_BUILD}</CodeBlock>
      </div>
    </section>
  );
}

export default function Home() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout
      title="Home"
      description={`${siteConfig.title} — full codebase documentation for the Windows Live Movie Maker 2012 (Sundance) source reconstruction.`}>
      <HomepageHeader />
      <main>
        <Stats />
        <Features />
        <Families />
        <BuildSample />
      </main>
    </Layout>
  );
}
