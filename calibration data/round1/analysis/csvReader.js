"use strict";

var fs = require('fs');

exports.parseCSV = function(file, divider) {
  if (!divider) divider = ','
  let data = fs.readFileSync(file);
  let reg = new RegExp('\r', 'g');
  let content = String(data)
    .replace(reg, '');

  let lines = content.split('\n');
  // read the first line
  let header = lines[0];
  let colsNames = header.split(divider);
  for(let i = 0; i < colsNames.length; i++) {
    colsNames[i] = colsNames[i].trim();
  }
  let returnValue = [];
  for (let i = 1; i < lines.length; i++) {
    let line = lines[i];
    if (line.trim() !== header.trim()) {
      let values = line.split(divider);
      if (values.length !== colsNames.length) continue;
      for (let j = 0; j < colsNames.length; j++) {
        if (!isNaN(values[j])) { // convert numbers into numbers
          values[j] = Number(values[j]);
        }
        if (!returnValue[colsNames[j]]) returnValue[colsNames[j]] = []
        returnValue[colsNames[j]].push(values[j])
      }
    }
  }
  return returnValue;
};
