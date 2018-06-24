"use strict";

var parser = require('./csvReader.js');
var stats = require('./stats.js');

var plotly = require('plotly')("DarioSalvi", "1AbAwLx0rKJLIssMq4nt");

var smr = require('smr');

var reference1 = parser.parseCSV('../../round 1/data/reference.csv', ',');
var data1 = parser.parseCSV('../../round 1/data/AIRSENSE.CSV', ' ');
var reference2 = parser.parseCSV('../data/reference.csv', ',');
var data2 = parser.parseCSV('../data/AIRSENSE.CSV', ',');

// parse dates and create Unix timestamps in reference 1
var refTS1 = []
for(let i = 0; i < reference1['TimeStamp'].length; i++) {
  reference1['TimeStamp'][i] = new Date(reference1['TimeStamp'][i]);
  refTS1[i] = reference1['TimeStamp'][i].getTime()
}

// in reference 2 it's slightly different
// Columns: Date,Time,NO2CNC1,NOXCNC1,NOCNC1
var refTS2 = []
reference2['TimeStamp'] = [] // create a new column called TimeStamp
for(let i = 0; i < reference2['Date'].length; i++) {
  let parseddate = new Date(reference2['Date'][i] + ' ' + reference2['Time'][i] + ':00');
  reference2['TimeStamp'][i] = parseddate
  refTS2[i] = reference2['TimeStamp'][i].getTime()
}

// adjust timestamps in round 1
// start date	18/05/2017
// start time	15:53:00 (at reference)
var startDate = Date.parse('05/18/2017 15:53:00')
var dataTS1 = []
for(let i = 0; i < data1['timestamp'].length; i++) {
  data1['timestamp'][i] = new Date(data1['timestamp'][i] + startDate);
  dataTS1[i] = data1['timestamp'][i].getTime()
}

// round 2
// start date	10/01/2018
// start time	11:23:21 (at reference)
var startDate = Date.parse('01/10/2018 11:23:21')
var dataTS2 = []
for(let i = 0; i < data2['timestamp'].length; i++) {
  data2['timestamp'][i] = new Date(data2['timestamp'][i] + startDate);
  dataTS2[i] = data2['timestamp'][i].getTime()
}

// create an extra data column for the Aphasense sensor
data1['rawNO2_Alpha'] = []
for(let i = 0; i < data1['rawNO2_Alpha1'].length; i++) {
  data1['rawNO2_Alpha'][i] = data1['rawNO2_Alpha1'][i] - data1['rawNO2_Alpha2'][i];
}

// Reference: Date,Time,NO2CNC1,NOXCNC1,NOCNC1
// Data: timestamp, temperature, humidity, pressure, custom_MICS, ratio_NH3,
// ratio_CO, ratio_NO2, nh3, co, no2, c3h8, c4h10, ch4, h2, c2h5oh,
// alpha1, alpha2, alphadiff, specdiff
function plotAll2() {
  // plot all data
  let traceref1 = {
    x: reference2['TimeStamp'],
    y: reference2['NOXCNC1'],
    name: 'NOX',
    type: "scatter",
    visible: 'legendonly'
  };
  let traceref2 = {
    x: reference2['TimeStamp'],
    y: reference2['NO2CNC1'],
    name: 'NO2',
    type: "scatter"
  };
  let traceref3 = {
    x: reference2['TimeStamp'],
    y: reference2['NOCNC1'],
    name: 'NO',
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedatahum = {
    x: data2['timestamp'],
    y: data2['humidity'],
    name: 'humidity',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedatatemp = {
    x: data2['timestamp'],
    y: data2['temperature'],
    name: 'temperature',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedatapress = {
    x: data2['timestamp'],
    y: data2['pressure'],
    name: 'pressure',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedatamics = {
    x: data2['timestamp'],
    y: data2['custom_MICS'],
    name: 'Mics',
    yaxis: "y2",
    type: "scatter"
  };
  let tracedataspec = {
    x: data2['timestamp'],
    y: data2['specdiff'],
    name: 'SPEC',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedataalpha1 = {
    x: data2['timestamp'],
    y: data2['alpha1'],
    name: 'AlphaSense1',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedataalpha2 = {
    x: data2['timestamp'],
    y: data2['alpha2'],
    name: 'AlphaSense2',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedataalpha = {
    x: data2['timestamp'],
    y: data2['alphadiff'],
    name: 'AlphaSense',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };

  let alldata = [traceref1, traceref2, traceref3, tracedatahum, tracedatatemp, tracedatapress, tracedatamics, tracedataspec, tracedataalpha1, tracedataalpha2, tracedataalpha];
  let layout = {
    title: "Data plot",
    yaxis: { title: 'ref (ppb)' },
    yaxis2: {
      title: 'data',
      overlaying: "y",
      side: "right"
    }
  };
  let graphOptions = {layout: layout, filename: "calibration-round-2", fileopt: "overwrite"};
  plotly.plot(alldata, graphOptions, function (err, msg) {
    console.log(msg);
  });
}

//plotAll2();
//https://plot.ly/~DarioSalvi/21/data-plot/#/

// interpolate data using reference
let alignedRef1 = [];
for (let dim in reference1) {
  alignedRef1[dim] = [];
}
let alignedRef2 = [];
for (let dim in reference2) {
  alignedRef2[dim] = [];
}
let alignedData1 = [];
for (let dim in data1) {
  alignedData1[dim] = [];
}
let alignedData2 = [];
for (let dim in data2) {
  alignedData2[dim] = [];
}


function isRefZeros(reference, index){
  let isZeros = true;
  for (let dim in reference) {
    if (dim != 'TimeStamp' && reference[dim][index] !== 0){
      isZeros = false;
    }
  }
  return isZeros;
}

function align(reference, refTS, alignedRef, data, dataTS, alignedData) {
  for (let i = 0; i < refTS.length; i++) {
    if(!isRefZeros(reference, i)) {
      // find the sample in data before and after
      let before = -1;
      let after = -1;
      for (let j = 0; (j < dataTS.length) && (dataTS[j] < refTS[i]); j++) before = j;
      for (let j = dataTS.length - 1; (j >= 0) && (dataTS[j] > refTS[i]); j--) after = j;
      if ((before >=0) && (after >=0)) {
        for (let dim in reference) {
          alignedRef[dim].push(reference[dim][i])
        }
        // interpolate
        for (let dim in data) {
          let currTS = refTS[i];
          let beforeTS = dataTS[before];
          let afterTS = dataTS[after];
          if (dim == 'timestamp'){
            alignedData[dim].push(reference['TimeStamp'][i]);
          } else {
            let interpolated = data[dim][before] + ((currTS - beforeTS) * (data[dim][after] - data[dim][before]) / (afterTS - beforeTS));
            alignedData[dim].push(interpolated);
          }
        }
      }
    }
  }
}

align(reference1, refTS1, alignedRef1, data1, dataTS1, alignedData1)
align(reference2, refTS2, alignedRef2, data2, dataTS2, alignedData2)
console.log('Aligned samples: ' + alignedRef2['NO2CNC1'].length + ' starting at ' + alignedRef2['TimeStamp'][0] + ' ending at ' + alignedRef2['TimeStamp'][alignedRef1['TimeStamp'].length-1])


// compute correlations matrix
console.log('correlations');
console.log('        NO2,\tNOX,\tNO');
// Mics
let no2_mics = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['custom_MICS']);
let nox_mics = stats.correlation(alignedRef2['NOXCNC1'], alignedData2['custom_MICS']);
let no_mics = stats.correlation(alignedRef2['NOCNC1'], alignedData2['custom_MICS']);
console.log('Mics    ' + no2_mics.toFixed(2) + ',\t' + nox_mics.toFixed(2) + ',\t' + no_mics.toFixed(2));
// SPEC
let no2_spec = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['specdiff']);
let nox_spec = stats.correlation(alignedRef2['NOXCNC1'], alignedData2['specdiff']);
let no_spec = stats.correlation(alignedRef2['NOCNC1'], alignedData2['specdiff']);
console.log('SPEC    ' + no2_spec.toFixed(2) + ',\t' + nox_spec.toFixed(2) + ',\t' + no_spec.toFixed(2));
// alpha1
let no2_alpha1 = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['alpha1']);
let nox_alpha1 = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['alpha1']);
let no_alpha1 = stats.correlation(alignedRef2['NOCNC1'], alignedData2['alpha1']);
console.log('Alpha1  ' + no2_alpha1.toFixed(2) + ',\t' + nox_alpha1.toFixed(2) + ',\t' + no_alpha1.toFixed(2));
// alpha2
let no2_alpha2 = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['alpha2']);
let nox_alpha2 = stats.correlation(alignedRef2['NOXCNC1'], alignedData2['alpha2']);
let no_alpha2 = stats.correlation(alignedRef2['NOCNC1'], alignedData2['alpha2']);
console.log('Alpha2  ' + no2_alpha2.toFixed(2) + ',\t' + nox_alpha2.toFixed(2) + ',\t' + no_alpha2.toFixed(2));
// alpha
let no2_alpha = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['alphadiff']);
let nox_alpha = stats.correlation(alignedRef2['NOXCNC1'], alignedData2['alphadiff']);
let no_alpha = stats.correlation(alignedRef2['NOCNC1'], alignedData2['alphadiff']);
console.log('Alpha d ' + no2_alpha.toFixed(2) + ',\t' + nox_alpha.toFixed(2) + ',\t' + no_alpha.toFixed(2));
// mq2
let no2_mq2 = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['specdiff']);
let nox_mq2 = stats.correlation(alignedRef2['NOXCNC1'], alignedData2['specdiff']);
let no_mq2 = stats.correlation(alignedRef2['NOCNC1'], alignedData2['specdiff']);
console.log('SPEC d  ' + no2_mq2.toFixed(2) + ',\t' + nox_mq2.toFixed(2) + ',\t' + no_mq2.toFixed(2));
// temp
let no2_temp = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['temperature']);
let nox_temp = stats.correlation(alignedRef2['NOXCNC1'], alignedData2['temperature']);
let no_temp = stats.correlation(alignedRef2['NOCNC1'], alignedData2['temperature']);
console.log('Temp    ' + no2_temp.toFixed(2) + ',\t' + nox_temp.toFixed(2) + ',\t' + no_temp.toFixed(2));
// hum
let no2_hum = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['humidity']);
let nox_hum = stats.correlation(alignedRef2['NOXCNC1'], alignedData2['humidity']);
let no_hum = stats.correlation(alignedRef2['NOCNC1'], alignedData2['humidity']);
console.log('Hum     ' + no2_hum.toFixed(2) + ',\t' + nox_hum.toFixed(2) + ',\t' + no_hum.toFixed(2));
// pressure
let no2_press = stats.correlation(alignedRef2['NO2CNC1'], alignedData2['pressure']);
let nox_press = stats.correlation(alignedRef2['NOXCNC1'], alignedData2['pressure']);
let no_press = stats.correlation(alignedRef2['NOCNC1'], alignedData2['pressure']);
console.log('Pressur ' + no2_press.toFixed(2) + ',\t' + nox_press.toFixed(2) + ',\t' + no_press.toFixed(2));

// denoise & re-sample
var avgTime = 30 * 60 * 1000;
function denoise(alignedData, alignedRef) {
  let newtimestamps;
  for (let dim in alignedData) {
    if (dim != 'timestamp'){
      var newarray = stats.averageTime(alignedData[dim], alignedData['timestamp'], avgTime);
      newtimestamps = newarray.timestamps;
      alignedData[dim] = newarray.samples;
    }
  }
  alignedData['timestamp'] = newtimestamps;
  for (let dim in alignedRef) {
    if (dim != 'TimeStamp'){
      var newarray = stats.averageTime(alignedRef[dim], alignedRef['TimeStamp'], avgTime);
      newtimestamps = newarray.timestamps;
      alignedRef[dim] = newarray.samples;
    }
  }
  alignedRef['TimeStamp'] = newtimestamps
}

denoise(alignedData1, alignedRef1)
denoise(alignedData2, alignedRef2)

console.log('After denoising samples 2: ' + alignedRef2['NO2CNC1'].length + ' and ' + alignedData2['timestamp'].length )

function computecoeffs(alignedData, alignedRef, inputs, maxDegrees, output) {
  let numx = maxDegrees.reduce(function(sum, value) {
    return sum + value;
  }, 0);

  let regression = new smr.Regression({ numX: numx, numY: 1 })
  for (let i = 0; i < alignedData['timestamp'].length; i++) {
    let x = []
    for (let j=0; j < inputs.length; j++) {
      let input = inputs[j];
      let maxdegree = maxDegrees[j];
      x.push(alignedData[input][i])
      for (let d=2; d <= maxdegree; d++) {
        x.push(Math.pow(alignedData[input][i], d));
      }
    }
    regression.push({ x: x, y: [ alignedRef[output][i] ] });
  }

  return regression;
}

function regress(alignedData, alignedRef, inputs, output, regression, regressed, errors) {
  for (let i = 0; i < alignedData['timestamp'].length; i++) {
    let x = []
    for (let j=0; j < inputs.length; j++) {
      let input = inputs[j];
      let maxdegree = maxDegrees[j];
      x.push(alignedData[input][i])
      for (let d=2; d <= maxdegree; d++) {
        x.push(Math.pow(alignedData[input][i], d));
      }
    }
    var regressedSample = regression.hypothesize({ x: x })[0];
    regressed.push( regressedSample );
    errors.push(Math.abs(regressedSample - alignedRef[output][i]))
  }
}

// regression on dataset 1, mics only scenario
var inputs = ['humidity', 'temperature',  'rawNO2_Mics'];
var maxDegrees = [3, 3, 3];
var output = 'NO2CNC1-AVG(PPB)';

var regression1 = computecoeffs(alignedData1, alignedRef1, inputs, maxDegrees, output)

let regressed = []
var errors = []
inputs = ['humidity', 'temperature',  'custom_MICS'];
output = 'NO2CNC1';


// regress round 2 using coefficients from round 1 !!!
regress(alignedData2, alignedRef2, inputs, output, regression1, regressed, errors)

// compute correlations matrix
console.log('correlations');
console.log('        NO2,\tNOX,\tNO');
let no2_regr = stats.correlation(alignedRef2['NO2CNC1'], regressed);
let nox_regr = stats.correlation(alignedRef2['NOXCNC1'], regressed);
let no_regr = stats.correlation(alignedRef2['NOCNC1'], regressed);
console.log('Regress ' + no2_regr.toFixed(2) + ',\t' + nox_regr.toFixed(2) + ',\t' + no_regr.toFixed(2));

console.log('Avg abs error: ' + stats.mean(errors).toFixed(2) + ' ppb');
console.log('Median abs error: ' + stats.median(errors).toFixed(2) + ' ppb');
console.log('Max abs error: ' + stats.max(errors).toFixed(2) + ' ppb');

function plotCrossRegress(){
  let trace1 = {
    x: alignedRef2['TimeStamp'],
    y: alignedRef2['NO2CNC1'],
    name: 'NO2',
    type: "scatter"
  };
  let trace2 = {
    x: alignedData2['timestamp'],
    y: regressed,
    name: 'regressed',
    type: "scatter"
  };
  let alldata = [trace1, trace2];
  let layout = {
    title: "Data plot",
    yaxis: {title: 'ppb'}
  };
  let graphOptions = {layout: layout, filename: 'round2-regressed-with-round1', fileopt: "overwrite"};
  plotly.plot(alldata, graphOptions, function (err, msg) {
    console.log(msg);
  });
}

//plotCrossRegress()


// regress round 2 with its data
// Data: timestamp, temperature, humidity, pressure, custom_MICS, ratio_NH3,
// ratio_CO, ratio_NO2, nh3, co, no2, c3h8, c4h10, ch4, h2, c2h5oh,
// alpha1, alpha2, alphadiff, specdiff
inputs = ['humidity', 'temperature', 'pressure', 'custom_MICS', 'alpha1', 'alpha2', 'alphadiff', 'specdiff'];
maxDegrees = [3, 3, 3, 3, 3, 3, 3, 3];
output = 'NO2CNC1';

regressed = []
errors = []

var regression2 = computecoeffs(alignedData2, alignedRef2, inputs, maxDegrees, output)
regress(alignedData2, alignedRef2, inputs, output, regression2, regressed, errors)

// compute correlations matrix
console.log('correlations');
console.log('        NO2,\tNOX,\tNO');
no2_regr = stats.correlation(alignedRef2['NO2CNC1'], regressed);
nox_regr = stats.correlation(alignedRef2['NOXCNC1'], regressed);
no_regr = stats.correlation(alignedRef2['NOCNC1'], regressed);
console.log('Regress ' + no2_regr.toFixed(2) + ',\t' + nox_regr.toFixed(2) + ',\t' + no_regr.toFixed(2));

console.log('Avg abs error: ' + stats.mean(errors).toFixed(2) + ' ppb');
console.log('Median abs error: ' + stats.median(errors).toFixed(2) + ' ppb');
console.log('Max abs error: ' + stats.max(errors).toFixed(2) + ' ppb');

function plotRound2Regress(){
  let trace1 = {
    x: alignedRef2['TimeStamp'],
    y: alignedRef2['NO2CNC1'],
    name: 'NO2',
    type: "scatter"
  };
  let trace2 = {
    x: alignedData2['timestamp'],
    y: regressed,
    name: 'regressed',
    type: "scatter"
  };
  let alldata = [trace1, trace2];
  let layout = {
    title: "Data plot",
    yaxis: {title: 'ppb'}
  };
  let graphOptions = {layout: layout, filename: 'round2-regressed', fileopt: "overwrite"};
  plotly.plot(alldata, graphOptions, function (err, msg) {
    console.log(msg);
  });
}

plotRound2Regress()


// sensitiviy analysis:

// only mics: correlation down to 0.8
// mics + alpha: is the same
// mic + spec stays at 0.9 (slightly worse)
// in other words SPEC and alphasense add the same information
// no pressure: doesn't change anything, so it doesn't add any valuable information
