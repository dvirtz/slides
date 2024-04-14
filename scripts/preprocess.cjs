const { readFile } = require('fs').promises;
const path = require('path');
const os = require('os')

const LINE_SEPARATOR = os.EOL;
const FILE_REF_REGEX = /^FILE: (.+)$/;
const SVG_START = /<svg.*>/;
const SVG_END = /<\/svg>/;
const SVG_ID = /id="([^"]+)"/;
const SVG_URL = /url\(#([^)]+)\)/;
const SLIDES = /^# Slides/;
const SLIDE_REF = /\.md\)/mg;

const loadFileContent = async (filePath, options, svgState) => {
  const contents = await readFile(path.join(options.initialDir, filePath), 'utf-8');
  if (path.extname(filePath) === '.md') {
    return await preprocessMarkdown(contents, options, svgState);
  }
  return contents;
};

const processLine = async (line, options, svgState) => {
  for (const m of (line.match(FILE_REF_REGEX) || []).slice(1)) {
    return await loadFileContent(m, options, svgState);
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

const preprocessMarkdown = async (markdown, options, svgState) => {
  const lines = markdown.split(LINE_SEPARATOR);
  const processedLines = await Promise.all(lines.map(line => processLine(line, options, svgState)));
  return processedLines.join(LINE_SEPARATOR);
}

const preprocess = async (markdown, options) => {
  var svgState = {
    insideSvg: false,
    svgCounter: 0
  };

  if (markdown.match(SLIDES) && options.static) {
    return markdown.replace(SLIDE_REF, '.html)');
  }

  options.initialDir = path.join(path.dirname(__dirname), 'slides');
  let res = await preprocessMarkdown(markdown, options, svgState);
  return res;
}

module.exports = preprocess;
