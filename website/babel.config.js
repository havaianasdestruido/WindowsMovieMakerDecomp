/**
 * Babel configuration for the Docusaurus site.
 *
 * Docusaurus >= 3.10 ships its Babel preset as `@docusaurus/babel/preset`;
 * older 3.x releases exposed it through `@docusaurus/preset-classic`.
 * Resolve whichever is available so the site builds against any installed
 * 3.x version.
 */
const docusaurusBabelPreset = (() => {
  try {
    return require.resolve('@docusaurus/babel/preset');
  } catch {
    return require.resolve('@docusaurus/preset-classic');
  }
})();

module.exports = {
  presets: [docusaurusBabelPreset],
};
