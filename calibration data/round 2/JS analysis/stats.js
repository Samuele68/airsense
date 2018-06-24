"use strict";

exports.mean = function(array) {
  let averaged = 0;
  for(let i = 1; i < array.length; i++) {
    averaged += array[i];
  }
  return averaged / array.length;
}

exports.median = function(array) {
  array.concat().sort( function(a,b) {return a - b;} );
  var half = Math.floor(array.length/2);
  if (array.length % 2) return array[half];
  else return (array[half-1] + array[half]) / 2.0;
}

exports.max = function(array) {
  return Math.max.apply(null, array);
}

exports.min = function(array) {
  return Math.min.apply(null, array);
}

exports.std = function(array) {
  let accum = 0;
  let avg = exports.mean(array)
  for(let i = 1; i < array.length; i++) {
    accum += Math.pow((array[i] - avg), 2)
  }
  accum /= array.length;
  accum = Math.sqrt(accum);
  return accum;
}

exports.movingAverage = function(array, buffer) {
  var retval = []
  for(let i = 1; i < array.length; i++) {
    let averaged = 0;
    for (let j = 0 ; j < (i < buffer ? i : buffer); j++) {
      averaged += array[i - j];
    }
    averaged /= buffer;
    retval[i] = averaged;
  }
  return retval;
}

exports.movingAverageTime = function(array, timestamps, time) {
  var retval = []
  for (var i = 0; i < array.length; i++) {
    var accum = 0;
    var j = 0;
    var timediff = timestamps[i].getTime() - timestamps[i - j].getTime()
    while ((i - j >= 0) && (timestamps[i].getTime() - timestamps[i - j].getTime() <= time)) {
      accum += array[i - j];
      j ++;
    }
    retval[i] = accum / j;
  }
  return retval;
}

exports.averageTime = function(array, timestamps, time) {
  var retval = {
    samples: [],
    timestamps: []
  }
  var i = 0;
  while (i < array.length) {
    var j = i;
    var accum = 0;
    while ((j < array.length) && (timestamps[j].getTime() - timestamps[i].getTime() <= time)) {
      accum += array[j];
      j++;
    }
    accum /= (j - i);
    retval.samples.push(accum);
    retval.timestamps.push(timestamps[j]);
    i = j;
  }
  return retval;
}


exports.correlation = function(array1, array2) {
  var mean1 = exports.mean(array1);
  var mean2 = exports.mean(array2);
  var nom = 0;
  var s1 = 0;
  var s2 = 0;
  for (let i=0; i < array1.length; i++) {
    let p1 = (array1[i] - mean1)
    let p2 = (array2[i] - mean2)
    nom += p1 * p2;
    s1 += Math.pow(p1, 2)
    s2 += Math.pow(p2, 2)
  }
  s1 = Math.sqrt(s1);
  s2 = Math.sqrt(s2);
  return nom / (s1 * s2);
}
