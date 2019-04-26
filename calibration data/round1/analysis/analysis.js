"use strict";

var parser = require('./csvReader.js');
var stats = require('./stats.js');

var plotly = require('plotly')("DarioSalvi", "1AbAwLx0rKJLIssMq4nt");

var smr = require('smr');

var reference = parser.parseCSV('../data/reference.csv', ',');
var data = parser.parseCSV('../data/AIRSENSE.CSV', ' ');

// parse dates and create Unix timestamps in reference
var refTS = []
for(let i = 0; i < reference['TimeStamp'].length; i++) {
  reference['TimeStamp'][i] = new Date(reference['TimeStamp'][i]);
  refTS[i] = reference['TimeStamp'][i].getTime()
}

// adjust timestamps in data
// start date	18/05/2017
// start time	15:53:00
var startDate = Date.parse('05/18/2017 15:53:00')
var dataTS = []
for(let i = 0; i < data['timestamp'].length; i++) {
  data['timestamp'][i] = new Date(data['timestamp'][i] + startDate);
  dataTS[i] = data['timestamp'][i].getTime()
}

// create an extra data column for the Aphasense sensor
data['rawNO2_Alpha'] = []
for(let i = 0; i < data['rawNO2_Alpha1'].length; i++) {
  data['rawNO2_Alpha'][i] = data['rawNO2_Alpha1'][i] + data['rawNO2_Alpha2'][i];
}

function plotAll(data) {
  // plot all data
  let traceref1 = {
    x: reference['TimeStamp'],
    y: reference['NOXCNC1-AVG(PPB)'],
    name: 'NOX',
    type: "scatter",
    visible: 'legendonly'
  };
  let traceref2 = {
    x: reference['TimeStamp'],
    y: reference['NO2CNC1-AVG(PPB)'],
    name: 'NO2',
    type: "scatter"
  };
  let traceref3 = {
    x: reference['TimeStamp'],
    y: reference['NOCNC1-AVG(PPB)'],
    name: 'NO',
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedatahum = {
    x: data['timestamp'],
    y: data['humidity'],
    name: 'humidity',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedatatemp = {
    x: data['timestamp'],
    y: data['temperature'],
    name: 'temperature',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedatamics = {
    x: data['timestamp'],
    y: data['rawNO2_Mics'],
    name: 'Mics',
    yaxis: "y2",
    type: "scatter"
  };
  let tracedataspec = {
    x: data['timestamp'],
    y: data['rawNO2_SPEC'],
    name: 'SPEC',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedataalpha1 = {
    x: data['timestamp'],
    y: data['rawNO2_Alpha1'],
    name: 'AlphaSense1',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedataalpha2 = {
    x: data['timestamp'],
    y: data['rawNO2_Alpha2'],
    name: 'AlphaSense2',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedataalpha = {
    x: data['timestamp'],
    y: data['rawNO2_Alpha'],
    name: 'AlphaSense',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedatamq2 = {
    x: data['timestamp'],
    y: data['rawmq2'],
    name: 'MQ2',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };
  let tracedatadust = {
    x: data['timestamp'],
    y: data['dustratio'],
    name: 'dust',
    yaxis: "y2",
    type: "scatter",
    visible: 'legendonly'
  };

  let alldata = [traceref1, traceref2, traceref3, tracedatahum, tracedatatemp, tracedatamics, tracedataspec, tracedataalpha1, tracedataalpha2, tracedataalpha, tracedatamq2, tracedatadust];
  let layout = {
    title: "Data plot",
    yaxis: { title: 'ref (ppb)' },
    yaxis2: {
      title: 'data',
      overlaying: "y",
      side: "right"
    }
  };
  let graphOptions = {layout: layout, filename: "multiple-axes-double", fileopt: "overwrite"};
  plotly.plot(alldata, graphOptions, function (err, msg) {
    console.log(msg);
  });
}

//plotAll(data);


// interpolate data using reference
let alignedRef = [];
for (let dim in reference) {
  alignedRef[dim] = [];
}
let alignedData = [];
for (let dim in data) {
  alignedData[dim] = [];
}

function isRefZeros(index){
  let isZeros = true;
  for (let dim in reference) {
    if (dim != 'TimeStamp' && reference[dim][index] !== 0){
      isZeros = false;
    }
  }
  return isZeros;
}

for (let i = 0; i < refTS.length; i++) {
  if(!isRefZeros(i)) {
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

console.log('Aligned samples: ' + alignedRef['NO2CNC1-AVG(PPB)'].length + ' starting at ' + alignedRef['TimeStamp'][0] + ' ending at ' + alignedRef['TimeStamp'][alignedRef['TimeStamp'].length-1])

// compute correlations matrix
console.log('correlations');
console.log('        NO2,\tNOX,\tNO');
// Mics
let no2_mics = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['rawNO2_Mics']);
let nox_mics = stats.correlation(alignedRef['NOXCNC1-AVG(PPB)'], alignedData['rawNO2_Mics']);
let no_mics = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], alignedData['rawNO2_Mics']);
console.log('Mics    ' + no2_mics.toFixed(2) + ',\t' + nox_mics.toFixed(2) + ',\t' + no_mics.toFixed(2));
// SPEC
let no2_spec = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['rawNO2_SPEC']);
let nox_spec = stats.correlation(alignedRef['NOXCNC1-AVG(PPB)'], alignedData['rawNO2_SPEC']);
let no_spec = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], alignedData['rawNO2_SPEC']);
console.log('SPEC    ' + no2_spec.toFixed(2) + ',\t' + nox_spec.toFixed(2) + ',\t' + no_spec.toFixed(2));
// alpha1
let no2_alpha1 = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['rawNO2_Alpha1']);
let nox_alpha1 = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['rawNO2_Alpha1']);
let no_alpha1 = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], alignedData['rawNO2_Alpha1']);
console.log('Alpha1  ' + no2_alpha1.toFixed(2) + ',\t' + nox_alpha1.toFixed(2) + ',\t' + no_alpha1.toFixed(2));
// alpha2
let no2_alpha2 = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['rawNO2_Alpha2']);
let nox_alpha2 = stats.correlation(alignedRef['NOXCNC1-AVG(PPB)'], alignedData['rawNO2_Alpha2']);
let no_alpha2 = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], alignedData['rawNO2_Alpha2']);
console.log('Alpha2  ' + no2_alpha2.toFixed(2) + ',\t' + nox_alpha2.toFixed(2) + ',\t' + no_alpha2.toFixed(2));
// alpha
let no2_alpha = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['rawNO2_Alpha']);
let nox_alpha = stats.correlation(alignedRef['NOXCNC1-AVG(PPB)'], alignedData['rawNO2_Alpha']);
let no_alpha = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], alignedData['rawNO2_Alpha']);
console.log('Alpha   ' + no2_alpha.toFixed(2) + ',\t' + nox_alpha.toFixed(2) + ',\t' + no_alpha.toFixed(2));
// mq2
let no2_mq2 = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['rawmq2']);
let nox_mq2 = stats.correlation(alignedRef['NOXCNC1-AVG(PPB)'], alignedData['rawmq2']);
let no_mq2 = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], alignedData['rawmq2']);
console.log('MQ2     ' + no2_mq2.toFixed(2) + ',\t' + nox_mq2.toFixed(2) + ',\t' + no_mq2.toFixed(2));
// dust
let no2_dustratio = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['dustratio']);
let nox_dustratio = stats.correlation(alignedRef['NOXCNC1-AVG(PPB)'], alignedData['dustratio']);
let no_dustratio = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], alignedData['dustratio']);
console.log('Dust    ' + no2_dustratio.toFixed(2) + ',\t' + nox_dustratio.toFixed(2) + ',\t' + no_dustratio.toFixed(2));
// temp
let no2_temp = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['temperature']);
let nox_temp = stats.correlation(alignedRef['NOXCNC1-AVG(PPB)'], alignedData['temperature']);
let no_temp = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], alignedData['temperature']);
console.log('Temp    ' + no2_temp.toFixed(2) + ',\t' + nox_temp.toFixed(2) + ',\t' + no_temp.toFixed(2));
// hum
let no2_hum = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], alignedData['humidity']);
let nox_hum = stats.correlation(alignedRef['NOXCNC1-AVG(PPB)'], alignedData['humidity']);
let no_hum = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], alignedData['humidity']);
console.log('Hum     ' + no2_hum.toFixed(2) + ',\t' + nox_hum.toFixed(2) + ',\t' + no_hum.toFixed(2));

// denoise & re-sample
var avgTime = 30 * 60 * 1000;
var newtimestamps;
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
console.log('After denoising samples: ' + alignedRef['NO2CNC1-AVG(PPB)'].length + ' and ' + alignedData['timestamp'].length )

// normalise inputs
// for (let dim in alignedData) {
//   if (dim != 'timestamp'){
//     let std = stats.std(alignedData[dim]);
//     for (let i = 0; i < alignedData['timestamp'].length; i++) {
//       alignedData[dim][i] = (alignedData[dim][i]) / std;
//     }
//   }
// }

// regression
// mics only scenario:
// var inputs = ['rawNO2_Mics', 'humidity', 'temperature'];
// var maxDegrees = [3, 3, 3]
// best combined scenario
var inputs = ['humidity', 'temperature',  'rawNO2_Mics'];
var maxDegrees = [3, 3, 3];
var output = 'NO2CNC1-AVG(PPB)';

let numx = maxDegrees.reduce(function(sum, value) {
  return sum + value;
}, 0);
var regression = new smr.Regression({ numX: numx, numY: 1 })
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

var coeffs = regression.calculateCoefficients();
console.log(coeffs)

var regressed = []
var errors = []
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
  errors.push(Math.abs(regressedSample - alignedRef['NO2CNC1-AVG(PPB)'][i]))
}


// regressed
// compute correlations matrix
console.log('correlations');
console.log('        NO2,\tNOX,\tNO');
let no2_regr = stats.correlation(alignedRef['NO2CNC1-AVG(PPB)'], regressed);
let nox_regr = stats.correlation(alignedRef['NOXCNC1-AVG(PPB)'], regressed);
let no_regr = stats.correlation(alignedRef['NOCNC1-AVG(PPB)'], regressed);
console.log('Regressed ' + no2_regr.toFixed(2) + ',\t' + nox_regr.toFixed(2) + ',\t' + no_regr.toFixed(2));

console.log('Avg abs error: ' + stats.mean(errors).toFixed(2) + ' ppb');
console.log('Median abs error: ' + stats.median(errors).toFixed(2) + ' ppb');
console.log('Max abs error: ' + stats.max(errors).toFixed(2) + ' ppb');

var trace1 = {
  x: alignedRef['TimeStamp'],
  y: alignedRef['NO2CNC1-AVG(PPB)'],
  name: 'NO2',
  type: "scatter"
};
var trace2 = {
  x: alignedData['timestamp'],
  y: regressed,
  name: 'regressed',
  type: "scatter"
};
var data = [trace1, trace2];
var layout = {
  title: "Data plot",
  yaxis: {title: 'ppb'}
};
var graphOptions = {layout: layout, filename: 'NO2-regressed', fileopt: "overwrite"};
plotly.plot(data, graphOptions, function (err, msg) {
  console.log(msg);
});
