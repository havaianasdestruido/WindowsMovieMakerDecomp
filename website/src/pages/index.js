import React from 'react';
import Link from '@docusaurus/Link';
import useBaseUrl from '@docusaurus/useBaseUrl';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import Heading from '@theme/Heading';
import CodeBlock from '@theme/CodeBlock';
import {
  ArrowUpRight,
  Bug,
  Cube,
  Dna,
  FileArrowUp,
  Flask,
  RocketLaunch,
} from '@phosphor-icons/react';
import Reveal from '@site/src/components/Reveal';

const STATS = [
  {value: '29', label: 'CMake build targets'},
  {value: '375+', label: 'Reconstructed source files'},
  {value: '1,360', label: 'RTTI classes mapped'},
  {value: '116', label: 'Contract tests green'},
  {value: '23', label: 'Original bugs preserved'},
  {value: '65', label: 'Analysis modules'},
];

const FEATURES = [
  {
    key: 'cell--a',
    icon: Dna,
    title: 'Faithful reconstruction',
    body: 'Windows Live Movie Maker 2012 rebuilt from binary analysis as C++14, ATL and WTL source, matching the original MSVC 11.0 binaries.',
    image: true,
  },
  {
    key: 'cell--b',
    icon: FileArrowUp,
    title: 'Export parity, enforced',
    body: 'Every DLL exports the same names, ordinals and decorations as the reference binary. tools/diff_exports.py gates the export surface on every change.',
    tint: true,
  },
  {
    key: 'cell--c',
    icon: Flask,
    title: 'Contract-tested behavior',
    body: 'A 116-check ctypes suite pins HRESULTs, GUIDs, factory semantics and file-format details across every DLL surface.',
    chips: ['PASS=116', 'FAIL=0'],
  },
  {
    key: 'cell--d',
    icon: Cube,
    title: 'Disciplined stub design',
    body: 'Telemetry is inert-but-safe, parsers are bounds-checked, credentials are DPAPI-encrypted. Every stub category is documented and intentional.',
  },
  {
    key: 'cell--e',
    icon: Bug,
    title: 'Quirks kept on purpose',
    body: '23 original bugs and anomalies, from the single-instance mutex to the null CLSID, are reproduced and cataloged in QUIRKS.md.',
  },
  {
    key: 'cell--f',
    icon: RocketLaunch,
    title: 'It launches',
    body: 'MovieMaker.exe boots, opens its window and runs the DirectUI and Ribbon pipeline on modern Windows with a VS2022 toolchain.',
  },
];

const FAMILIES = [
  {
    to: '/docs/modules/application',
    count: '4 targets',
    title: 'Application',
    body: 'The launcher, the MovieMakerCore.dll engine, localization resources and the preview client.',
  },
  {
    to: '/docs/modules/dui-engine',
    count: '7 targets',
    title: 'DirectUI engine layer',
    body: 'UXCore, uxctl, MediaCatalog, ProjectManager, TimelineEngine, GPURenderer and PlaybackEngine.',
  },
  {
    to: '/docs/modules/media-foundation',
    count: '5 targets',
    title: 'Media Foundation',
    body: 'WLMFDS, WLMFReadWrite, WLXMP4Parser, WLXCodecHost and the WLXTranscode helper.',
  },
  {
    to: '/docs/modules/pipeline-effects',
    count: '6 targets',
    title: 'Pipeline and effects',
    body: 'WLXPipeline, WLXPipetran, WLXSlideshow, WLXPhotoCinematic, WLXVideoTrim and WLXMovieLibrary.',
  },
  {
    to: '/docs/modules/platform',
    count: '7 targets',
    title: 'Platform services',
    body: 'WLXPhotoBase, publishing, face recognition, metadata, identity and the inert telemetry pair.',
  },
];

const STEPS = [
  {verb: 'Configure', desc: 'Generate the VS2022 x86 solution with CMake.'},
  {verb: 'Build', desc: 'Compile the Debug configuration of all 29 targets.'},
  {verb: 'Run', desc: 'Launch build/bin/Debug/MovieMaker.exe and open a project.'},
  {verb: 'Verify', desc: 'Run the ctypes suite against the built DLLs.'},
];

const SAMPLE_BUILD = `# Configure + build (Win32 / VS2022)
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug

# Run the reconstructed app
build\\bin\\Debug\\MovieMaker.exe

# Contract suite (behavioral regression oracle)
$env:WMMR_DLL_DIR = "build\\bin\\Debug"
python32\\python.exe tests\\mmr-python\\run_tests.py   # PASS=116 FAIL=0`;

function Hero() {
  const heroImg = useBaseUrl('/img/hero-film.jpg');
  return (
    <header className="wmmr-hero">
      <div className="wmmr-hero-inner">
        <div className="wmmr-hero-copy">
          <p className="wmmr-eyebrow wmmr-rise" style={{'--wmmr-i': 0}}>
            Windows Live Movie Maker 2012 · codename Sundance
          </p>
          <Heading as="h1" className="wmmr-hero-title wmmr-rise" style={{'--wmmr-i': 1}}>
            Reconstructed, <span className="wmmr-amber">verified,</span> documented.
          </Heading>
          <p className="wmmr-hero-sub wmmr-rise" style={{'--wmmr-i': 2}}>
            Build 16.4.3528.0331 (codename Sundance), rebuilt from binary analysis into C++14,
            ATL and WTL source. Parity pinned by 116 contract checks.
          </p>
          <div className="wmmr-hero-ctas wmmr-rise" style={{'--wmmr-i': 3}}>
            <Link className="wmmr-btn wmmr-btn--primary" to="/docs/intro">
              Get started
            </Link>
            <Link className="wmmr-btn wmmr-btn--ghost" to="/docs/modules/overview">
              Browse modules
            </Link>
          </div>
        </div>
        <Reveal className="wmmr-hero-media" delay={150}>
          <figure className="wmmr-hero-fig">
            <img
              src={heroImg}
              alt="35mm film strip curled on a dark workbench, warmed by amber light from the left edge"
              width="768"
              height="1376"
              loading="eager"
              fetchPriority="high"
              decoding="async"
            />
            <figcaption className="wmmr-hero-cap">Build 16.4.3528.0331</figcaption>
          </figure>
        </Reveal>
      </div>
    </header>
  );
}

function Stats() {
  return (
    <section className="wmmr-stats" aria-label="Project at a glance">
      <ul className="wmmr-stats-grid">
        {STATS.map((stat, i) => (
          <Reveal as="li" key={stat.label} className="wmmr-stat" delay={i * 60}>
            <span className="wmmr-stat-value">{stat.value}</span>
            <span className="wmmr-stat-label">{stat.label}</span>
          </Reveal>
        ))}
      </ul>
    </section>
  );
}

function FeatureCell({feature, index}) {
  const Icon = feature.icon;
  const bentoImg = useBaseUrl('/img/bento-inspection.jpg');
  const className = `wmmr-cell wmmr-${feature.key}`;
  return (
    <Reveal className={className} delay={index * 70}>
      {feature.image ? (
        <>
          <img
            className="wmmr-cell-img"
            src={bentoImg}
            alt="Film negatives glowing on a backlit inspection table beside a precision caliper"
            width="1584"
            height="672"
            loading="lazy"
            decoding="async"
          />
          <div className="wmmr-cell-scrim" aria-hidden="true" />
          <div className="wmmr-cell-body-onimg">
            <div className="wmmr-cell-title">{feature.title}</div>
            <p>{feature.body}</p>
          </div>
        </>
      ) : (
        <>
          <div className="wmmr-cell-head">
            <Icon size={22} weight="regular" aria-hidden="true" />
            <div className="wmmr-cell-title">{feature.title}</div>
          </div>
          <p className="wmmr-cell-body">{feature.body}</p>
          {feature.chips ? (
            <div className="wmmr-cell-chips">
              {feature.chips.map((chip) => (
                <code key={chip} className="wmmr-chip">
                  {chip}
                </code>
              ))}
            </div>
          ) : null}
        </>
      )}
    </Reveal>
  );
}

function Features() {
  return (
    <section className="wmmr-section">
      <Reveal>
        <Heading as="h2" className="wmmr-section-title">
          Why this reconstruction holds up
        </Heading>
        <p className="wmmr-section-sub">
          Measured not by lines of code, but by verified behavioral parity against the original
          binaries.
        </p>
      </Reveal>
      <div className="wmmr-bento">
        {FEATURES.map((feature, i) => (
          <FeatureCell key={feature.title} feature={feature} index={i} />
        ))}
      </div>
    </section>
  );
}

function Families() {
  return (
    <section className="wmmr-section wmmr-section--split">
      <Reveal className="wmmr-section-side">
        <Heading as="h2" className="wmmr-section-title">
          The codebase, by module family
        </Heading>
        <p className="wmmr-section-sub">
          29 build targets in five families, from the launcher EXE to the inert telemetry pair.
          Every module has its own page.
        </p>
      </Reveal>
      <Reveal className="wmmr-families" delay={90}>
        <ul>
          {FAMILIES.map((family) => (
            <li key={family.to}>
              <Link to={family.to} className="wmmr-family">
                <span className="wmmr-family-main">
                  <span className="wmmr-family-title">{family.title}</span>
                  <span className="wmmr-family-body">{family.body}</span>
                </span>
                <span className="wmmr-family-meta">
                  <code className="wmmr-chip wmmr-chip--accent">{family.count}</code>
                  <ArrowUpRight size={18} weight="regular" className="wmmr-family-arrow" aria-hidden="true" />
                </span>
              </Link>
            </li>
          ))}
        </ul>
      </Reveal>
    </section>
  );
}

function Build() {
  return (
    <section className="wmmr-section wmmr-build">
      <Reveal className="wmmr-build-copy">
        <Heading as="h2" className="wmmr-section-title">
          Build it yourself
        </Heading>
        <p className="wmmr-section-sub">
          MSVC only, Win32 x86 only: the same toolchain constraints as the 2012 release.
        </p>
        <ol className="wmmr-steps">
          {STEPS.map((step, i) => (
            <li key={step.verb} className="wmmr-step">
              <span className="wmmr-step-index">{String(i + 1).padStart(2, '0')}</span>
              <span className="wmmr-step-text">
                <span className="wmmr-step-verb">{step.verb}</span>
                <span className="wmmr-step-desc">{step.desc}</span>
              </span>
            </li>
          ))}
        </ol>
      </Reveal>
      <Reveal className="wmmr-build-code" delay={100}>
        <CodeBlock language="powershell" title="from a VS2022 developer prompt">
          {SAMPLE_BUILD}
        </CodeBlock>
      </Reveal>
    </section>
  );
}

export default function Home() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout
      title="Home"
      description={`${siteConfig.title}: full codebase documentation for the Windows Live Movie Maker 2012 (Sundance) source reconstruction.`}>
      <Hero />
      <main>
        <Stats />
        <Features />
        <Families />
        <Build />
      </main>
    </Layout>
  );
}
