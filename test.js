import * as sokol_audio from "./build/sokol_audio.so";
import * as dsp from "./build/soundplug.so";
import * as os from "os";

var samplerate = 44100;
var channels = 1;
var buffer_frames = 2048;

var saw = dsp.make_hex();
var sawbuffer = new ArrayBuffer(2048);
saw.sample(sawbuffer, 500/samplerate);
console.log(sawbuffer);
console.log("HELLO")

while (1) {
  os.sleep(100);
  console.log("woke");
}