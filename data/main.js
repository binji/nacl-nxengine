"use strict";

var gameDataUrl = 'http://www.cavestory.org/downloads/cavestoryen.zip';
var gameDataFile = 'cavestoryen.zip';

function status(message) {
  var messageEl = document.getElementById('message');
  messageEl.style.display = 'block';
  messageEl.textContent = '[ ' + message + ' ]';
  console.log(message);
}

function clearStatus() {
  document.getElementById('message').style.display = 'none';
}

function downloadGameData(fileSystem) {
  function onProgress(event) {
    if (event.lengthComputable) {
      var percentComplete = event.loaded * 100 / event.total;
      status('downloaded ' + percentComplete.toFixed(0) + '%');
    }
  }

  function onError(event) {
    status('error downloading ' + gameDataUrl);
  }

  function onLoad(event) {
    var data = event.target.response;

    // Write the data into the filesystem.
    console.log('Opening ' + gameDataFile + ' to save downloaded data.');
    fileSystem.root.getFile(gameDataFile, {create:true},
        function onGetFileSuccess(fileEntry) {
          console.log('Creating file writer...');
          fileEntry.createWriter(function (fileWriter) {
            fileWriter.addEventListener('writeend', function (event) {
              console.log('Wrote to ' + gameDataFile);
              createEmbed();
            });

            fileWriter.addEventListener('error', function (error) {
              status('unable to write ' + gameDataFile);
            });

            console.log('Writing data...');
            fileWriter.write(data);
          });
        },
        function onGetFileError(error) {
          status('unable to create ' + gameDataFile);
        });
  }

  // Download the game data.
  console.log('Downloading game data from ' + gameDataUrl);
  var xhr = new XMLHttpRequest();
  xhr.addEventListener('progress', onProgress, false);
  xhr.addEventListener('load', onLoad, false);
  xhr.addEventListener('error', onError, false);
  xhr.open('GET', gameDataUrl, true);
  xhr.responseType = 'blob';
  xhr.send();
}

function createEmbed() {
  clearStatus();

  var manifest = chrome.runtime.getManifest();
  var name = manifest.name;
  var isDebug = name.indexOf('debug') !== -1;
  var nmf = isDebug ? 'nx_debug.nmf' : 'nx_release.nmf';

  var embed = document.createElement('embed');
  embed.setAttribute('type', 'application/x-nacl');
  embed.setAttribute('src', nmf);
  embed.setAttribute('width', '640');
  embed.setAttribute('height', '480');
  embed.setAttribute('ps_verbosity', '0');
  embed.setAttribute('ps_stderr', '/dev/stderr');

  var listener = document.getElementById('listener');

  listener.addEventListener('error', function() {
    embed.parentNode.removeChild(embed);
    status('error loading module');
  }, true);

  listener.addEventListener('crash', function() {
    embed.parentNode.removeChild(embed);
    status('module crashed');
  }, true);

  listener.addEventListener('message', function (e) {
    if (typeof(e.data) !== 'string') {
      return;
    }

    if (e.data === 'quit') {
      window.close();
    }
  }, true);

  listener.appendChild(embed);
}

document.addEventListener('DOMContentLoaded', function () {
  // Check to see if the data is in the html5 filesystem.
  status('opening filesystem');
  window.webkitRequestFileSystem(window.PERSISTENT, 0,
      function onFilesystemRequestSuccess(fileSystem) {
        // Has the file already been downloaded?
        status('opening file ' + gameDataFile);
        fileSystem.root.getFile(gameDataFile, {},
            function onGetFileSuccess(fileEntry) {
              // File's already there, start the NEXE.
              createEmbed();
            },
            function onGetFileError(err) {
              status(gameDataFile + ' not found');
              downloadGameData(fileSystem);
            });
      },
      function onFilesystemRequestError() {
        status('unable to open filesystem');
      });
});
