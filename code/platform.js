//- Globals
var HEAPU8 = null;
var canvas = null;

//- Helper functions
function readHeapString(ptr, length)
{
	if (length === 0 || !ptr) return '';
 var hasutf = 0;
	for (var hasUtf = 0, t, i = 0; !length || i != length; i++)
	{
		t = HEAPU8[((ptr)+(i))>>0];
		if (t == 0 && !length) break;
		hasUtf |= t;
	}
	if (!length) length = i;
	if (hasUtf & 128)
	{
		for(var r=HEAPU8,o=ptr,p=ptr+length,F=String.fromCharCode,e,f,i,n,C,t,a,g='';;)
		{
			if(o==p||(e=r[o++],!e)) return g;
			128&e?(f=63&r[o++],192!=(224&e)?(i=63&r[o++],224==(240&e)?e=(15&e)<<12|f<<6|i:(n=63&r[o++],240==(248&e)?e=(7&e)<<18|f<<12|i<<6|n:(C=63&r[o++],248==(252&e)?e=(3&e)<<24|f<<18|i<<12|n<<6|C:(t=63&r[o++],e=(1&e)<<30|f<<24|i<<18|n<<12|C<<6|t))),65536>e?g+=F(e):(a=e-65536,g+=F(55296|a>>10,56320|1023&a))):g+=F((31&e)<<6|f)):g+=F(e);
		}
	}
	// split up into chunks, because .apply on a huge string can overflow the stack
	for (var ret = '', curr; length > 0; ptr += 1024, length -= 1024)
		ret += String.fromCharCode.apply(String, HEAPU8.subarray(ptr, ptr + Math.min(length, 1024)));
	return ret;
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

//- Platform
async function main()
{
 var env =
 {
  LogMessage: function(length, string)
  {
   let message = readHeapString(string, length);
   console.log(message);
  },

  floor: Math.floor,
  ceil: Math.ceil,
  sqrt: Math.sqrt,
  pow: Math.pow,
  fmod: function(a, b) { 
   let result = a%b;
   return result;
  },
  cos: Math.cos,
  acos: Math.acos,
  fabs: Math.abs,
  round: Math.round,

  JS_DrawText: function(length, string, x, y) {
   let text = readHeapString(string, length);
   const context = canvas.getContext("2d");
   context.font = "16px serif";
   context.fillText("hello", x, y);
  },
 };

 let wasm_instance = 0;

 let { instance } = await WebAssembly.instantiateStreaming(
   fetch("./game.wasm"), {env:env}
 );

 wasm_instance = instance;
 HEAPU8 = new Uint8Array(wasm_instance.exports.memory.buffer);

 let reload = false;
 const socket = new WebSocket('ws://localhost:1234');
 socket.onmessage = async function incoming(message)
 {
  if(message.data === "reload")
  {
   reload = true;
  }
 };

 //- Image
 const width = wasm_instance.exports.GetBufferWidth();
 const height = wasm_instance.exports.GetBufferHeight();
 const bytes_per_pixel = wasm_instance.exports.GetBytesPerPixel();

 canvas = document.getElementById("graphics-canvas");
 canvas.width = width;
 canvas.height = height;
 const context = canvas.getContext("2d");

 let buffer_address = wasm_instance.exports.GlobalImageBuffer.value;
 let image = new ImageData(
                new Uint8ClampedArray(wasm_instance.exports.memory.buffer,
                                      buffer_address,
                                      bytes_per_pixel * width * height),
                width,
                height);

 //- Mouse Input
 var mouse_x = -1;
 var mouse_y = -1;
 var mouse_down = false;
 var mouse_clicked = 0;

 console.log(canvas);

 document.addEventListener('pointermove', (event) =>
 {
  // Update element position here or handle drawing
  var x_offset = (window.innerWidth - width)/2;
  var y_offset = (window.innerHeight - height)/2;

  mouse_x = event.clientX - x_offset;
  mouse_y = event.clientY - y_offset;
 });

 document.onmousedown = function(event)
 {
  mouse_down = true;
 };

 document.onmouseup = function(event)
 {
  mouse_down = false;
 };
 
 let update_hz = 30;
 let target_seconds_for_frame = 1/update_hz;

 {
 let input_element = document.body.querySelector("input");
 let rect = canvas.getBoundingClientRect();
 let x = 8;
 let y = 10;
 let width = 142;
 let height = 42;
 input_element.style.position = `absolute`;
 input_element.style.left = `${rect.left + x}px`;
 input_element.style.top = `${rect.top + y}px`;
 input_element.style.width = `${width}px`;
 input_element.style.height = `${height}px`;
 }

 //- Game loop
 let running = true;
 let end_counter = 0;
 let work_time_elapsed = 0;
 while(running)
 {
  if(reload)
  {
   let { instance } = await WebAssembly.instantiateStreaming(
     fetch("./game.wasm"), {env:env}
   );
   HEAPU8 = new Uint8Array(instance.exports.memory.buffer);
   image = new ImageData(
            new Uint8ClampedArray(instance.exports.memory.buffer,
                                  instance.exports.GlobalImageBuffer.value,
                                  bytes_per_pixel * width * height),
            width, height);
   wasm_instance = instance;
   reload = false;
  }

  let last_counter = performance.now();

  // NOTE(luca): We need one frame of lag here because otherwise we cannot call
  // canvas drawing functions in `UpdateAndRender()` since they will be overwritten
  // by the `context.putImageData()` call.
  context.putImageData(image, 0, 0);

  wasm_instance.exports.UpdateAndRender(width, height, bytes_per_pixel,
                                        target_seconds_for_frame, 
                                        mouse_down, mouse_x, mouse_y);


  await new Promise(requestAnimationFrame);
 }

}

window.onload = main;
