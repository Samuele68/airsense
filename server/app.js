'use strict'

const express = require('express')
const bodyParser = require('body-parser')
const fs = require('fs')

const app = express()
app.use(bodyParser.text())
const port = 80

const dataFile = 'data/data.csv'

var lastData = {}

app.use(express.static('public'))
app.use(express.static('data'))
app.get('/test', (req, res) => res.send('Hello fuckers!'))

app.post('/airsense/data', (req, res) => {
	let line = req.body
	line = new Date().toISOString() + ', ' + line + '\n'
	lastData = {
		timestamp: new Date()
	}
	fs.appendFile(dataFile, line, function (err) {
		if (err) res.sendStatus(500)
		else res.sendStatus(200)
	})
})

app.get('/airsense/data', (req, res) => {
	fs.readFile(dataFile, function (err, data) {
		if (err) res.sendStatus(500)
		else res.type('text/plain').send(data)
	})
})

app.get('/airsense/status', (req, res) => {
	res.send(lastData)
})

// The following is used for tests, to be removed

const alexFile = 'data/alex.csv'

app.post('/alex/data', (req, res) => {
	let line = req.body
	line = new Date().toISOString() + ', ' + line + '\n'
	fs.appendFile(alexFile, line, function (err) {
		if (err) res.sendStatus(500)
		else res.sendStatus(200)
	})
})

app.listen(port, () => console.log(`Airsense server listening on port ${port}!`))
