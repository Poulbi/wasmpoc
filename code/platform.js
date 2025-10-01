//- Global variables
let HEAPU8;

//- Helper functions

function ReadHeapString(ptr, length)
{
	if (length === 0 || !ptr) return '';
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
 let env =
 {
  LogMessage: function(length, message)
  {
   let messageJS = ReadHeapString(message, length);
   console.log(messageJS);
  },
 };

 const { instance } = await WebAssembly.instantiateStreaming(
   fetch("./game.wasm"), {env:env}
 );
 HEAPU8 = new Uint8Array(instance.exports.memory.buffer);

 const width = instance.exports.GetBufferWidth();
 const height = instance.exports.GetBufferHeight();
 const bytes_per_pixel = instance.exports.GetBytesPerPixel();

 const canvas = document.getElementById("graphics-canvas");
 const ctx = canvas.getContext("2d");
 canvas.width = width;
 canvas.height = height;
 //
 //while(true)
 //{
 //}
 //
 const buffer_address = instance.exports.Buffer.value;
 const image = new ImageData(
                new Uint8ClampedArray(instance.exports.memory.buffer,
                                      buffer_address,
                                      bytes_per_pixel * width * height),
                width,
                height);

 let prev = null;
 function frame(timestamp)
 {
  let dt = (timestamp - prev)*0.001;
  prev = timestamp;

  // TODO(luca): How to get this???
  let update_hz = 144;
  instance.exports.RenderGradient(width, height, 1/update_hz);
  ctx.putImageData(image, 0, 0);
  window.requestAnimationFrame(frame);
 }

 window.requestAnimationFrame((timestamp) => {
  prev = timestamp;
  window.requestAnimationFrame(frame);
 });
}


window.onload = main;
