const SLIDES = /^# Slides/
const FILE_REF = /^FILE: (.+)$/gm;
const SLIDE_REF = /\.md\)/m;

module.exports = async (markdown, options) => {
  if (markdown.match(SLIDES) && options.static) {
    return markdown.replace(SLIDE_REF, '.html)');
  }

  return markdown.replace(FILE_REF, function (p1) {
    return readFile(join(options.includeDir, p1));
  });
}