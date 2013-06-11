chrome.app.runtime.onLaunched.addListener(function (launchData) {
  chrome.app.window.create('index.html', {
    width: 640,
    height: 480,
    resizable: false
  });
});
