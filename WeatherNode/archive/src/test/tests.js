// Required for exposing test results to the Sauce Labs API.
// Can be removed when the following issue is fixed:
// https://github.com/axemclion/grunt-saucelabs/issues/84
QUnit.done(function (details) {
	window.global_test_results = details;
});


var lifecycle = {
	teardown: function () {
	}
};

test('simple value', function () {
	expect(1);
	ok(true, 'should return value');
});
