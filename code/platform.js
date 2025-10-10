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

//- Platform
async function main()
{
 const socket = new WebSocket('ws://localhost:1234');
 socket.onmessage = function incoming(message)
 {
  if(message.data === "reload")
  {
   window.location.reload();
  }
 };
 
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

 const { instance } = await WebAssembly.instantiateStreaming(
   fetch("./game.wasm"), {env:env}
 );
 HEAPU8 = new Uint8Array(instance.exports.memory.buffer);

 //- Image
 const width = instance.exports.GetBufferWidth();
 const height = instance.exports.GetBufferHeight();
 const bytes_per_pixel = instance.exports.GetBytesPerPixel();

 const canvas = document.getElementById("graphics-canvas");
 const ctx = canvas.getContext("2d");
 canvas.width = width;
 canvas.height = height;

 const buffer_address = instance.exports.GlobalImageBuffer.value;
 const image = new ImageData(
                new Uint8ClampedArray(instance.exports.memory.buffer,
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
 
 //- Game loop
 let prev = 0;
 let timestamp = 0;
 let update_hz = 30;
 while(true)
 {
  let dt = (timestamp - prev)*0.001;
  prev = timestamp;

  instance.exports.UpdateAndRender(width, height, bytes_per_pixel, 1/update_hz, mouse_down, mouse_x, mouse_y);
  ctx.putImageData(image, 0, 0);

  await new Promise(requestAnimationFrame);
 }
}

window.onload = main;
