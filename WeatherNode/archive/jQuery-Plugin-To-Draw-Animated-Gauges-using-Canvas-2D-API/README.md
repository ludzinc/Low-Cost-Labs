# jquery.gauge [![Build Status](https://travis-ci.org/swehacker/jquery.gauge.svg)](https://travis-ci.org/swehacker/jquery.gauge) [![Issues](http://img.shields.io/github/issues/swehacker/jquery.gauge.svg)]( https://github.com/swehacker/jquery.gauge/issues )
A simple gauge.

## Installation

Include script *after* the jQuery library (unless you are packaging scripts somehow else):

```html
<script src="/path/to/jquery.gauge.js"></script>
```

**Do not include the script directly from GitHub (http://raw.github.com/...).** The file is being served as text/plain and as such being blocked
in Internet Explorer on Windows 7 for instance (because of the wrong MIME type). Bottom line: GitHub is not a CDN.

## Usage

Create a canvas in where you want to place your gauge:

```html
<canvas id="myGauge" width="300" height="300">
```

And in between script tag lookup the canvas and use gauge(value)
```javascript
$("#myGauge").gauge(70);
```

### Options
Key | Default | Description
----|---------|------------
type| default | Type of gauge: default or halfcircle
min | 0 | Minimum value, e.g temperature from -120 celsius
max | 100 | Maximum value to display
unit | % | Unit to be displayed after the value
font | 50px verdana | Font to be used for the values
color |  lightgreen | Color for the value and bar
bgcolor | #222 | Background color for the bar

```javascript
$("#myGauge").gauge(70, { unit: "Volts" })
```

## Contributing
Check out the [Contributing Guidelines](CONTRIBUTING.md)

## Authors
[Patrik Falk](https://github.com/swehacker)
