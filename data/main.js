"use strict";

var gameDataUrl = 'http://www.cavestory.org/downloads/cavestoryen.zip';
var gameDataFile = 'cavestoryen.zip';

function downloadGameData(fileSystem) {
  function onProgress(event) {
    if (event.lengthComputable) {
      var percentComplete = event.loaded * 100 / event.total;
      console.log('Downloaded ' + percentComplete.toFixed(0) + '%');
    }
  }

  function onError(event) {
    console.log('Error downloading file.');
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
              console.log('Unable to write ' + gameDataFile);
            });

            console.log('Writing data...');
            fileWriter.write(data);
          });
        },
        function onGetFileError(error) {
          console.log('Unable to create ' + gameDataFile);
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
  console.log('Launching NEXE.');

  var embed = document.createElement('embed');
  embed.setAttribute('type', 'application/x-nacl');
  embed.setAttribute('src', 'nx_debug.nmf');
  embed.setAttribute('width', '640');
  embed.setAttribute('height', '480');
  embed.setAttribute('ps_verbosity', '0');
  embed.setAttribute('ps_stderr', '/dev/stderr');

  var listener = document.getElementById('listener');
  listener.appendChild(embed);
}

document.addEventListener('DOMContentLoaded', function () {
  // Check to see if the data is in the html5 filesystem.
  console.log('Opening filesystem...');
  window.webkitRequestFileSystem(window.PERSISTENT, 0,
      function onFilesystemRequestSuccess(fileSystem) {
        // Has the file already been downloaded?
        console.log('Trying to get file ' + gameDataFile + '...');
        fileSystem.root.getFile(gameDataFile, {},
            function onGetFileSuccess(fileEntry) {
              // File's already there, start the NEXE.
              createEmbed();
            },
            function onGetFileError(err) {
              console.log(gameDataFile + ' not found.');
              downloadGameData(fileSystem);
            });
      },
      function onFilesystemRequestError(err) {
        console.log('Unable to open filesystem. error = ' + err);
      });
});
