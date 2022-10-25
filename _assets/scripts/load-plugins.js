options.plugins.splice(options.plugins.indexOf(RevealHighlight), 0, RevealCompilerExplorer);
if (typeof RevealEmbedTweet !== 'undefined') {
  options.plugins.push(RevealEmbedTweet);
}
if (typeof RevealTimeline !== 'undefined') {
  options.plugins.push(RevealTimeline);
}
if (typeof RevealPdf !== 'undefined') {
  options.plugins.push(RevealPdf)
}
