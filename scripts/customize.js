Reveal.on('ready', (event) => {
  $('ul > li > input[type=checkbox]').parent()
    .css("margin-left", "-1.5em")
    .css("list-style-type", "none");
  $('li > ul > li > input[type=checkbox]').parent()
    .css("margin-left", "-5.5em");
  $('li:has(.fragment[type=checkbox])')
    .attr('class', 'fragment')
    .attr('data-fragment-index', function () {
      var child = $(this).children('.fragment[type=checkbox]')
      var fragmentIndex = child.attr('data-fragment-index')
      child.removeAttr('data-fragment-index').removeClass('fragment')
      return fragmentIndex;
    });
  $('.aside').attr('data-background-color', "#bee4fd");
  $('.container code').attr('data-fragment-index', 0);
  $('.animated').addClass('should-animate').removeClass('animated');
  $(Reveal.getRevealElement()).prepend($('<a>', {
    class: 'github-fork-ribbon top-right fixed',
    href: 'https://github.com/dvirtz/slides',
    'data-ribbon': 'Fork me on GitHub'
  }));
  $('.chapter').addClass('r-stretch');
  if (Reveal.isSpeakerNotes()) {
    $('.github-fork-ribbon').css('visibility', 'hidden');
  }
  $('pre.split').each(function () {
    $(this).removeClass('split').css('width', '100%')
    if (!$(this).attr("style").includes('font-size')) {
      $(this).css('font-size', '0.35em');
    }
    $(this).children('code').addClass('split').css('column-count', $(this).prop('style').columnCount || 2).each(function () {
      const lastNotEmpty = $('tr td:nth-child(2)', this).filter(function () {
        return $(this).text().trim() !== '';
      }).get(-1);
      if (lastNotEmpty) {
        const index = $(lastNotEmpty.parentElement).index();
        $('tr td:nth-child(1)', this).slice(index + 1).hide();
      }
    });
    $(this).css('column-count', '');
  });
  // move quote cites right
  $('blockquote p:has(cite)').css('float', 'right');
  Reveal.sync();
});

Reveal.on('slidechanged', (event) => {
  $(event.currentSlide).find('.should-animate').addClass('animated');
  $(event.previousSlide).find('.should-animate').removeClass('animated');
  $(event.currentSlide).find('pre[data-auto-animate-target]')
    .on('transitionstart', function () {
      $(this).find('code').css('overflow', 'hidden')
    })
    .on('transitionend', function () {
      $(this).find('code').css('overflow', 'auto')
    })
  $(event.previousSlide).find('pre[data-auto-animate-target] code').css('overflow', 'hidden');
});

Reveal.on('overviewshown', event => {
  const gifs = $('div[style*="background-image"]').filter(function() {
    return $(this).css('background-image').includes('.gif');
  });
  
  gifs.filter(':not([frozen-background])').each(function () {
    var c = document.createElement('canvas');
    var w = c.width = $(this).width();
    var h = c.height = $(this).height();
    const image = new Image();
    image.onload = () => {
      c.getContext('2d').drawImage(image, 0, 0, w, h);
      $(this).attr('original-background', $(this).css('background-image'));
      $(this).attr('frozen-background', `url(${c.toDataURL("image/gif")})`);
      $(this).css('background-image', $(this).attr('frozen-background'));
    };
    image.src = $(this).css('background-image').match(/\((.*?)\)/)[1].replace(/('|")/g,'');
  });

  gifs.filter('[frozen-background]').css('background-image', function() {
    return $(this).attr('frozen-background');
  });
});

Reveal.on('overviewhidden', event => {
  $('div[original-background]').css('background-image', function() {
    return $(this).attr('original-background');
  });
});
