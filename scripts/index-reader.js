const { readFileSync } = require('fs');
const { readFile } = require('fs').promises;
const { join } = require('path');

const SLIDES = /^# Slides/
const FILE_REF = /^FILE: (.+)$/gm;
const SLIDE_REF = /- (.+)$/m;

const readIndex = async (markdown, options) => {
  if (markdown.match(SLIDES)) {
    return markdown.replace(SLIDE_REF, '[$1]($1/index.md)');
  }

  return markdown.replace(FILE_REF, function (p1) {
    return readFile(join(options.includeDir, p1));
  });
}

const fileListImpl = async (path, re) =>
  readFile(path, 'ascii').then(markdown =>
    [...markdown.matchAll(re)]
      .map(m => m[1]));

const fileListSyncImpl = (path, re) =>
  [...readFileSync(path, 'ascii').matchAll(re)]
    .map(m => m[1]);

module.exports = {
  readIndex,
  fileList: async (path) => fileListImpl(path, FILE_REF),
  slideList: async (path) => fileListImpl(path, SLIDE_REF),
  fileListSync: (path) => fileListSyncImpl(path, FILE_REF),
  slideListSync: (path) => fileListSyncImpl(path, SLIDE_REF)
}
