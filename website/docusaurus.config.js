// @ts-check
// Note: type annotations are only enabled for IDE support.
// There is no runtime type-checking or TS compilation step in this project.

const {themes} = require('prism-react-renderer');
const lightCodeTheme = themes.github;
const darkCodeTheme = themes.vsDark;

// Canonical hosting is the GitHub Pages project path at /docs/; local previews can
// override the base URL (e.g. WMMR_DOCS_BASE_URL=/docs/ or /).
const baseUrl = process.env.WMMR_DOCS_BASE_URL || '/WindowsMovieMakerDecomp/docs/';

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'WindowsMovieMakerDecomp Docs',
  tagline:
    'Windows Live Movie Maker 2012 (codename Sundance) reconstructed from binary analysis as C++14 / ATL / WTL source, and fully documented.',

  // GitHub Pages hosting (project site).
  url: 'https://havaianasdestruido.github.io',
  baseUrl,

  // Used by `docusaurus deploy` and GitHub Pages publishing.
  organizationName: 'havaianasdestruido',
  projectName: 'WindowsMovieMakerDecomp',

  onBrokenLinks: 'throw',

  favicon: 'img/favicon.svg',

  // GitHub Pages deployment of the docs site never includes the trailing hash.
  trailingSlash: false,

  // Even if you don't use internationalization, you can use this field to set
  // useful metadata like html lang.
  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },

  markdown: {
    mermaid: true,
    hooks: {
      onBrokenMarkdownLinks: 'warn',
    },
  },

  themes: ['@docusaurus/theme-mermaid'],

  presets: [
    [
      'classic',
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          path: 'docs',
          routeBasePath: '/',
          sidebarPath: require.resolve('./sidebars.js'),
          editUrl:
            'https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/edit/main/website/',
          showLastUpdateAuthor: false,
          showLastUpdateTime: false,
        },
        blog: false,
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      }),
    ],
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      image: 'img/logo.svg',
      colorMode: {
        defaultMode: 'dark',
        respectPrefersColorScheme: true,
      },
      navbar: {
        title: 'WMMR Docs',
        logo: {
          alt: 'WindowsMovieMakerDecomp logo',
          src: 'img/logo.svg',
          href: 'https://havaianasdestruido.github.io/WindowsMovieMakerDecomp/',
          target: '_self',
        },
        items: [
          {
            href: 'https://havaianasdestruido.github.io/WindowsMovieMakerDecomp/',
            label: '← Main Site',
            position: 'left',
            target: '_self',
          },
          {type: 'doc', docId: 'intro', label: 'Docs', position: 'left'},
          {type: 'doc', docId: 'architecture/overview', label: 'Architecture', position: 'left'},
          {type: 'doc', docId: 'modules/overview', label: 'Modules', position: 'left'},
          {type: 'doc', docId: 'testing/overview', label: 'Testing', position: 'left'},
          {type: 'doc', docId: 'methodology/reconstruction', label: 'Methodology', position: 'left'},
          {
            href: 'https://github.com/havaianasdestruido/WindowsMovieMakerDecomp',
            label: 'GitHub',
            position: 'right',
          },
        ],
      },
      footer: {
        style: 'dark',
        links: [
          {
            title: 'Docs',
            items: [
              {label: 'Introduction', to: '/'},
              {label: 'Getting Started', to: '/getting-started/overview'},
              {label: 'Architecture', to: '/architecture/overview'},
              {label: 'Module Reference', to: '/modules/overview'},
            ],
          },
          {
            title: 'Engineering',
            items: [
              {label: 'Testing', to: '/testing/overview'},
              {label: 'Methodology', to: '/methodology/reconstruction'},
              {label: 'Stub Design', to: '/methodology/stub-design'},
              {label: 'Preserved Quirks', to: '/methodology/quirks'},
            ],
          },
          {
            title: 'Repository',
            items: [
              {
                label: 'Main Site',
                href: 'https://havaianasdestruido.github.io/WindowsMovieMakerDecomp/',
                target: '_self',
              },
              {
                label: 'GitHub',
                href: 'https://github.com/havaianasdestruido/WindowsMovieMakerDecomp',
              },
              {
                label: 'Issue Tracker',
                href: 'https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues',
              },
              {
                label: 'Root README',
                href: 'https://github.com/havaianasdestruido/WindowsMovieMakerDecomp#readme',
              },
            ],
          },
        ],
        copyright: `Copyright © 2026 WindowsMovieMakerDecomp contributors. Reconstructed for research and interoperability purposes. Not affiliated with Microsoft.`,
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
        additionalLanguages: ['cpp', 'cmake', 'powershell', 'batch', 'ini'],
      },
      mermaid: {
        theme: {light: 'default', dark: 'dark'},
        options: {
          fontFamily: "'Geist', system-ui, sans-serif",
          wrap: true,
        },
      },
      announcementBar: {
        id: 'star-announcement',
        content:
          'Enjoying the reconstruction? Star <a target="_blank" rel="noopener noreferrer" href="https://github.com/havaianasdestruido/WindowsMovieMakerDecomp">the repository on GitHub</a>.',
        backgroundColor: 'var(--ifm-color-primary)',
        textColor: '#ffffff',
        isCloseable: true,
      },
      tableOfContents: {
        minHeadingLevel: 2,
        maxHeadingLevel: 4,
      },
      docs: {
        sidebar: {
          autoCollapseCategories: true,
        },
      },
    }),
};

module.exports = config;
