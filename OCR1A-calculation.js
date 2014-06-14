/**
 * compute OCR1A from BPM and division (javascript function)
 */

function getOCR1A(bpm, division, prescaler) {
	prescaler = prescaler || 1024;
	var masterClock = 16000000; // arduino master clock, 16MHz
	var notePerBeat = 4;        // resolution is quater-notes
	var OCR1A_val = (60 * masterClock) / (bpm * division * prescaler * notePerBeat);
	return OCR1A_val.toFixed(2);
}

for (var t = 30; t <= 300; t += 10) {
	var p = 8;
	console.log("	* │ " + t + " │ " + getOCR1A(t, 16, p) + " │ " + getOCR1A(t, 32, p) + " │ " + getOCR1A(t, 64, p) + " │ " + getOCR1A(t, 256, p) + " │ ");
}