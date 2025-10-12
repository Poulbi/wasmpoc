//- Global variables
var HEAPU8;

//- Helper functions
function ReadHeapString(ptr, length)
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
  LogMessage: function(length, message)
  {
   var messageJS = ReadHeapString(message, length);
   console.log(messageJS);
  },
  floor: Math.floor,
  ceil: Math.ceil,
  sqrt: Math.sqrt,
  pow: Math.pow,
  fmod: Math.fround, // Note: JavaScript doesn't have a native fmod function, so we use fround as a close approximation
  cos: Math.cos,
  acos: Math.acos,
  fabs: Math.abs, // Note: JavaScript doesn't have a native fabs function, so we use abs as a close approximation
  round: Math.round,
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

 const canvas = document.getElementById("graphics-canvas");
 const ctx = canvas.getContext("2d");
 canvas.width = width;
 canvas.height = height;

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

  wasm_instance.exports.UpdateAndRender(width, height, bytes_per_pixel,
                                        target_seconds_for_frame, 
                                        mouse_down, mouse_x, mouse_y);

  ctx.putImageData(image, 0, 0);

  await new Promise(requestAnimationFrame);
 }

}

window.onload = main;
