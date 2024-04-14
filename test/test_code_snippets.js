import { parseMarkdownFile, compile } from 'reveal-test';
import path from 'path';
import { cwd } from 'process';
import FileHound from 'filehound';

const snippets = await FileHound.create().path('slides').depth(1).ext('md').find()
  .map(async file => {
    const config = (() => {
      if (file.endsWith('number_string.md')) {
        return {
          compiler: 'g111',
          options: '-std=c++20 -O2 -march=haswell -Wall -Wextra -pedantic -Wno-unused-variable -Wno-unused-parameter'
        };
      }
      else if (file.includes('ranges_23')) {
        return {
          compiler: 'gsnapshot',
          options: '-std=c++23 -O2 -march=haswell -Wall -Wextra -pedantic -Wno-unused-variable -Wno-unused-parameter'
        };
      }
      else {
        return {
          compiler: 'g83'
        };
      }
    })();
    return {
      file: path.relative('slides', file),
      snippets: await parseMarkdownFile(path.join(cwd(), file), config)
    };
  })
  .filter(fileSnippets => fileSnippets.snippets.length > 0);


snippets.map(fileSnippets => {
  describe(`compile snippets from ${fileSnippets.file}`, function () {

    fileSnippets.snippets.forEach(function (codeSnippet, index) {
      it(`should compile snippet #${index} of ${fileSnippets.file}`, async function () {
        await compile(codeSnippet);
      });
    });
  });
});
