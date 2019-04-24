/*
Node sketch that parses the log files and generates a csv file
*/
'use strict'

var fs = require('fs'),
readline = require('readline')

if (process.argv.length <3) {
  console.log('usage: nodejs logparser FILENAME')
} else {
  let rd = readline.createInterface({
    input: fs.createReadStream(process.argv[2])
  })

  let csvOut = process.argv[2] + '.csv'
  if (fs.existsSync(csvOut)) fs.unlinkSync(csvOut)
  fs.appendFileSync(csvOut, 'Date,Time,NO2CNC1,NOXCNC1,NOCNC1\n', 'utf8');

  let currentStartDate

  rd.on('line', function(line) {
    let lineArgs = line.split(' ')
    if (lineArgs[0] === 'D') {
      if (lineArgs[1] === 'REPORT' && lineArgs[2] === '"CONC1"') {
        // FROM=01/04/2019
        let dateArgs = lineArgs[3].split('=')
        if (dateArgs.length > 1) {
          dateArgs = dateArgs[1].split('/')
          currentStartDate = new Date(Date.UTC(dateArgs[2], dateArgs[0] - 1, dateArgs[1], 0, 0, 0))
        }
      } else if (lineArgs.length > 3 && lineArgs[6] === 'CONC1:') {
        let timeArgs = lineArgs[2].split(':')
        if (timeArgs.length > 1 ) {
          let timestamp = new Date(Date.UTC(currentStartDate.getFullYear(), currentStartDate.getMonth(), currentStartDate.getDate(), timeArgs[1], timeArgs[2], 0))
          let NOCNC1 = lineArgs[11]
          let NOXCNC1 = lineArgs[13]
          let NO2CNC1 = lineArgs[15]
          let line = timestamp.getUTCFullYear() + '/' + (timestamp.getUTCMonth() + 1) + '/' + timestamp.getUTCDate() +
          ',' + timestamp.getUTCHours() + ':' + timestamp.getUTCMinutes() +
          ',' + NO2CNC1 + ',' + NOXCNC1 + ',' + NOCNC1 + '\n'
          fs.appendFileSync(csvOut, line, 'utf8')
        }
      }
    }
  })
}
