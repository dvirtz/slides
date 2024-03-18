const EmbedCode = {
    id: 'externalCode',
    init: (reveal) => {
        return Promise.all([...document.querySelectorAll('code[data-url]')].map((el) => {
            const url = el.getAttribute('data-url').trim();

            if (url === '') {            
                return;
            }

            function showError(error, showInline = true) {
                if (showInline) {
                    el.innerHTML = error;
                }

                console.error(error);
            }

            return fetch(url)
                .then((r) => {
                    if (r.ok !== true) {
                        showError(`code could not be loaded from ${url}: status ${r.status} returned`);
                        return;
                    }

                    return r.text();
                }).then((t) => {
                    let lines = t.split('\n');
                    let lineStart = el.getAttribute('data-line-start');
                    const lineStartDelimiter = el.getAttribute('data-line-start-delimiter') ?? '';
                    let lineEnd = el.getAttribute('data-line-end');
                    const lineEndDelimiter = el.getAttribute('data-line-end-delimiter') ?? '';
                    const beautify = !el.hasAttribute('data-no-beautify');
                    const includeDelimiters = el.hasAttribute('data-include-delimiters');

                    function getLineNumberThatContains(text) {
                        for (let i = 0; i < lines.length; i++) {
                            if (lines[i].includes(text)) {
                                // add 1 as we're using line numbers, not indices
                                return i + 1;
                            }
                        }

                        // none found
                        return -1;
                    }

                    if (lineStartDelimiter !== '') {
                        // search for start delimiter and set it's line (plus offset) as the line start
                        lineStart = getLineNumberThatContains(lineStartDelimiter);

                        if (lineStart > 0 && !includeDelimiters) {
                            lineStart++;
                        }
                    }

                    if (lineEndDelimiter !== '') {
                        // search for end delimiter and set it's line (plus offset) as the line end
                        lineEnd = getLineNumberThatContains(lineEndDelimiter);

                        if (lineEnd > 0 && !includeDelimiters) {
                            lineEnd--;
                        }
                    }

                    if (lineEnd !== null) {
                        lineEnd = parseInt(lineEnd);

                        if(isNaN(lineEnd) || lineEnd > lines.length || lineEnd < 0) {
                            showError(`invalid line end specified: ${lineEnd}`, false);
                        }

                        // end at line end
                        lines.splice(lineEnd, lines.length - lineEnd);
                    }

                    if (lineStart !== null) {
                        lineStart = parseInt(lineStart);

                        if(isNaN(lineStart) || lineStart < 0 || lineStart > lines.length || (lineEnd !== null && lineStart > lineEnd)) {
                            showError(`invalid line start specified: ${lineStart}`, false);
                        }

                        // start at line start
                        lines.splice(0, lineStart - 1);
                    }

                    if (beautify) {
                        // beautify loaded code
                        // e.g. remove similar indentation
                        const reg = new RegExp("^\\s+");
                        let minIndentation = Infinity;

                        for (let line of lines) {
                            if (line === '') {
                                // skip empty lines as these have no indentation but still allow beautifying
                                continue;
                            }

                            let matches = line.match(reg);

                            if (null === matches || matches.length === 0) {
                                // some line does not have any indentation, therefore cannot beautify
                                minIndentation = 0;
                                break;
                            }

                            if (matches[0].length < minIndentation) {
                                minIndentation = matches[0].length;
                            }
                        }

                        if (minIndentation > 0) {
                            // remove common indentation from all lines
                            lines = lines.map((l) => l.substring(minIndentation));
                        }
                    }

                    // join lines again
                    t = lines.join('\n');

                    el.textContent = t;
                })
                .catch((e) => {
                    // todo: add git url
                    showError(`code could not be loaded from ${url} as an exception occurred: ${e}\n\nCould it be that you're not loading the presentation via a (local) server?\nOtherwise, contact the developer or open an issue at https://github.com/befocken/revealjs-embed-code.`);
                });
        }));
    },
};