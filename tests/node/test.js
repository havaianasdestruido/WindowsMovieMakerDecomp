const ffi = require('ffi-napi');
const ref = require('ref-napi');

const dllPath = 'MovieMakerCore.dll';

// Assuming MovieMakerMain signature: int MovieMakerMain();
const int = ref.types.int;

const lib = ffi.Library(dllPath, {
  MovieMakerMain: [int, []]
});

console.log('Calling MovieMakerMain...');
const result = lib.MovieMakerMain();
console.log('Result:', result);
