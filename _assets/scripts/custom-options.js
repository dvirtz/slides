
function onF16orF17Pressed(event) {
  Reveal.configure({
    keyboard: {
      37: 'prev',
      39: 'next',
      127: 'right',
      128: 'left'
    }
  });
}

options.keyboard = {
  127: onF16orF17Pressed,
  128: onF16orF17Pressed
}
