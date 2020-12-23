const { readFileSync } = require('fs');
const path = require('path');
const { getInitialDir } = require('reveal-md/lib/config');

const LINE_SEPARATOR = '\n';
const FILE_REF_REGEX = /^FILE: (.+)$/;
const SVG_START = /<svg.*>/;
const SVG_END = /<\/svg>/;
const SVG_ID = /id="([^"]+)"/;
const SVG_URL = /url\(#([^)]+)\)/;
const SLIDES = /^# Slides/;
const SLIDE_REF = /\.md\)/m;

const loadFileContent = (filePath, options, svgState) => {
  const contents = readFileSync(path.join(options.initialDir, filePath), 'utf-8');
  if (path.extname(filePath) === '.md') {
    return preprocessMarkdown(contents, options, svgState);
  }
  return contents;
};

const processLine = (line, options, svgState) => {
  for (const m of (line.match(FILE_REF_REGEX) || []).slice(1)) {
    return loadFileContent(m, options, svgState);
  }

  if (svgState.insideSvg) {
    if (line.match(SVG_END)) {
      svgState.insideSvg = false;
      svgState.svgCounter = svgState.svgCounter + 1;
    } else {
      return line.replace(SVG_ID, `id="$1${svgState.svgCounter}"`).replace(SVG_URL, `url(#$1${svgState.svgCounter})`);
    }
  } else if (line.match(SVG_START)) {
    svgState.insideSvg = true;
  }

  return line;
}

const preprocessMarkdown = (markdown, options, svgState) => {
  return markdown
    .split(LINE_SEPARATOR)
    .map(line => processLine(line, options, svgState))
    .join(LINE_SEPARATOR);
}

const preprocess = async (markdown, options) => {
  var svgState = {
    insideSvg: false,
    svgCounter: 0
  };

  if (markdown.match(SLIDES) && options.static) {
    return markdown.replace(SLIDE_REF, '.html)');
  }

  options.initialDir = await getInitialDir();
  let res = preprocessMarkdown(markdown, options, svgState);
  return res;
}

module.exports = preprocess;
