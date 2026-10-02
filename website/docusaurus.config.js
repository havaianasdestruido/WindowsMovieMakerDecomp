// @ts-check
// Note: type annotations are only enabled for IDE support.
// There is no runtime type-checking or TS compilation step in this project.

const {themes} = require('prism-react-renderer');
const lightCodeTheme = themes.github;
const darkCodeTheme = themes.dracula;

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'WindowsMovieMakerDecomp',
  tagline:
    'Windows Live Movie Maker 2012 (codename Sundance) — reconstructed from binary analysis as C++14 / ATL / WTL source, and fully documented.',

  // GitHub Pages hosting (project site).
  url: 'https://havaianasdestruido.github.io',
  baseUrl: '/WindowsMovieMakerDecomp/',

  // Used by `docusaurus deploy` and GitHub Pages publishing.
  organizationName: 'havaianasdestruido',
  projectName: 'WindowsMovieMakerDecomp',

  onBrokenLinks: 'throw',

  favicon: 'img/favicon.svg',

  // GitHub Pages deployment of the docs site never includes the trailing hash.
  // Keep this in sync with .github/workflows/docs.yml.
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
          routeBasePath: 'docs',
          sidebarPath: require.resolve('./sidebars.js'),
          // Please change this to your repo.
          // Remove this to remove the "edit this page" links.
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
      // Replace with your project's social card
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
        },
        items: [
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
              {label: 'Introduction', to: '/docs/intro'},
              {label: 'Getting Started', to: '/docs/getting-started/overview'},
              {label: 'Architecture', to: '/docs/architecture/overview'},
              {label: 'Module Reference', to: '/docs/modules/overview'},
            ],
          },
          {
            title: 'Engineering',
            items: [
              {label: 'Testing', to: '/docs/testing/overview'},
              {label: 'Methodology', to: '/docs/methodology/reconstruction'},
              {label: 'Stub Design', to: '/docs/methodology/stub-design'},
              {label: 'Preserved Quirks', to: '/docs/methodology/quirks'},
            ],
          },
          {
            title: 'Repository',
            items: [
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
          fontFamily: 'system-ui, sans-serif',
          wrap: true,
        },
      },
      announcementBar: {
        id: 'star-announcement',
        content:
          '⭐ If you find this reconstruction useful, consider starring <a target="_blank" rel="noopener noreferrer" href="https://github.com/havaianasdestruido/WindowsMovieMakerDecomp">the repository on GitHub</a>!',
        backgroundColor: 'var(--ifm-color-primary)',
        textColor: '#111111',
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
