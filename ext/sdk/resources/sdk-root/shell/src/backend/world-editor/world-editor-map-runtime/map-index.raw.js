/**
 * GCC compiled flatbush package
 * @see https://github.com/mourner/flatbush
 *
 * ISC License
 *
 * Copyright (c) 2018, Vladimir Agafonkin
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
 var FlatQueue=function(){this.ids=[];this.values=[];this.length=0};FlatQueue.prototype.clear=function(){this.length=0};FlatQueue.prototype.push=function(e,d){var a=this.length++;this.ids[a]=e;for(this.values[a]=d;0<a;){var b=a-1>>1,c=this.values[b];if(d>=c)break;this.ids[a]=this.ids[b];this.values[a]=c;a=b}this.ids[a]=e;this.values[a]=d};
 FlatQueue.prototype.pop=function(){if(0!==this.length){var e=this.ids[0];this.length--;if(0<this.length){for(var d=this.ids[0]=this.ids[this.length],a=this.values[0]=this.values[this.length],b=this.length>>1,c=0;c<b;){var f=(c<<1)+1,h=f+1,g=this.ids[f],k=this.values[f],l=this.values[h];h<this.length&&l<k&&(f=h,g=this.ids[h],k=l);if(k>=a)break;this.ids[c]=g;this.values[c]=k;c=f}this.ids[c]=d;this.values[c]=a}return e}};FlatQueue.prototype.peek=function(){return this.ids[0]};
 FlatQueue.prototype.peekValue=function(){return this.values[0]};
 var ARRAY_TYPES=[Int8Array,Uint8Array,Uint8ClampedArray,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array],VERSION=3,Flatbush=function(e,d,a,b){void 0===d&&(d=16);void 0===a&&(a=Float64Array);if(void 0===e)throw Error("Missing required argument: numItems.");if(isNaN(e)||0>=e)throw Error("Unpexpected numItems value: "+e+".");this.numItems=+e;this.nodeSize=Math.min(Math.max(+d,2),65535);var c=e,f=c;this._levelBounds=[4*c];do c=Math.ceil(c/this.nodeSize),f+=c,this._levelBounds.push(4*
 f);while(1!==c);this.ArrayType=a||Float64Array;this.IndexArrayType=16384>f?Uint16Array:Uint32Array;c=ARRAY_TYPES.indexOf(this.ArrayType);var h=4*f*this.ArrayType.BYTES_PER_ELEMENT;if(0>c)throw Error("Unexpected typed array class: "+a+".");b&&b instanceof ArrayBuffer?(this.data=b,this._boxes=new this.ArrayType(this.data,8,4*f),this._indices=new this.IndexArrayType(this.data,8+h,f),this._pos=4*f,this.minX=this._boxes[this._pos-4],this.minY=this._boxes[this._pos-3],this.maxX=this._boxes[this._pos-2],
 this.maxY=this._boxes[this._pos-1]):(this.data=new ArrayBuffer(8+h+f*this.IndexArrayType.BYTES_PER_ELEMENT),this._boxes=new this.ArrayType(this.data,8,4*f),this._indices=new this.IndexArrayType(this.data,8+h,f),this._pos=0,this.minY=this.minX=Infinity,this.maxY=this.maxX=-Infinity,(new Uint8Array(this.data,0,2)).set([251,(VERSION<<4)+c]),(new Uint16Array(this.data,2,1))[0]=d,(new Uint32Array(this.data,4,1))[0]=e);this._queue=new FlatQueue};
 Flatbush.from=function(e){if(!(e instanceof ArrayBuffer))throw Error("Data must be an instance of ArrayBuffer.");var d=new Uint8Array(e,0,2),a=d[1];if(251!==d[0])throw Error("Data does not appear to be in a Flatbush format.");if(a>>4!==VERSION)throw Error("Got v"+(a>>4)+" data when expected v"+VERSION+".");d=(new Uint16Array(e,2,1))[0];var b=(new Uint32Array(e,4,1))[0];return new Flatbush(b,d,ARRAY_TYPES[a&15],e)};
 Flatbush.prototype.add=function(e,d,a,b){var c=this._pos>>2;this._indices[c]=c;this._boxes[this._pos++]=e;this._boxes[this._pos++]=d;this._boxes[this._pos++]=a;this._boxes[this._pos++]=b;e<this.minX&&(this.minX=e);d<this.minY&&(this.minY=d);a>this.maxX&&(this.maxX=a);b>this.maxY&&(this.maxY=b);return c};
 Flatbush.prototype.finish=function(){if(this._pos>>2!==this.numItems)throw Error("Added "+(this._pos>>2)+" items when expected "+this.numItems+".");if(this.numItems<=this.nodeSize)this._boxes[this._pos++]=this.minX,this._boxes[this._pos++]=this.minY,this._boxes[this._pos++]=this.maxX,this._boxes[this._pos++]=this.maxY;else{for(var e=this.maxX-this.minX,d=this.maxY-this.minY,a=new Uint32Array(this.numItems),b=0;b<this.numItems;b++){var c=4*b,f=this._boxes[c++],h=this._boxes[c++],g=this._boxes[c++];
 c=this._boxes[c++];a[b]=hilbert(Math.floor(65535*((f+g)/2-this.minX)/e),Math.floor(65535*((h+c)/2-this.minY)/d))}sort(a,this._boxes,this._indices,0,this.numItems-1,this.nodeSize);for(d=e=0;e<this._levelBounds.length-1;e++)for(a=this._levelBounds[e];d<a;){b=d;h=f=Infinity;c=g=-Infinity;for(var k=0;k<this.nodeSize&&d<a;k++)f=Math.min(f,this._boxes[d++]),h=Math.min(h,this._boxes[d++]),g=Math.max(g,this._boxes[d++]),c=Math.max(c,this._boxes[d++]);this._indices[this._pos>>2]=b;this._boxes[this._pos++]=
 f;this._boxes[this._pos++]=h;this._boxes[this._pos++]=g;this._boxes[this._pos++]=c}}};
 Flatbush.prototype.search=function(e,d,a,b,c){if(this._pos!==this._boxes.length)throw Error("Data not yet indexed - call index.finish().");for(var f=this._boxes.length-4,h=[],g=[];void 0!==f;){for(var k=Math.min(f+4*this.nodeSize,upperBound(f,this._levelBounds)),l=f;l<k;l+=4){var m=this._indices[l>>2]|0;a<this._boxes[l]||b<this._boxes[l+1]||e>this._boxes[l+2]||d>this._boxes[l+3]||(f<4*this.numItems?(void 0===c||c(m))&&g.push(m):h.push(m))}f=h.pop()}return g};
 Flatbush.prototype.neighbors=function(e,d,a,b,c){void 0===a&&(a=Infinity);void 0===b&&(b=Infinity);if(this._pos!==this._boxes.length)throw Error("Data not yet indexed - call index.finish().");var f=this._boxes.length-4,h=this._queue,g=[];for(b*=b;void 0!==f;){for(var k=Math.min(f+4*this.nodeSize,upperBound(f,this._levelBounds)),l=f;l<k;l+=4){var m=this._indices[l>>2]|0,n=axisDist(e,this._boxes[l],this._boxes[l+2]),p=axisDist(d,this._boxes[l+1],this._boxes[l+3]);n=n*n+p*p;f<4*this.numItems?(void 0===
 c||c(m))&&h.push(-m-1,n):h.push(m,n)}for(;h.length&&0>h.peek();){if(h.peekValue()>b)return h.clear(),g;g.push(-h.pop()-1);if(g.length===a)return h.clear(),g}f=h.pop()}h.clear();return g};function axisDist(e,d,a){return e<d?d-e:e<=a?0:e-a}function upperBound(e,d){for(var a=0,b=d.length-1;a<b;){var c=a+b>>1;d[c]>e?b=c:a=c+1}return d[a]}
 function sort(e,d,a,b,c,f){if(!(Math.floor(b/f)>=Math.floor(c/f))){for(var h=e[b+c>>1],g=b-1,k=c+1;;){do g++;while(e[g]<h);do k--;while(e[k]>h);if(g>=k)break;swap(e,d,a,g,k)}sort(e,d,a,b,k,f);sort(e,d,a,k+1,c,f)}}function swap(e,d,a,b,c){var f=e[b];e[b]=e[c];e[c]=f;e=4*b;f=4*c;var h=d[e],g=d[e+1],k=d[e+2],l=d[e+3];d[e]=d[f];d[e+1]=d[f+1];d[e+2]=d[f+2];d[e+3]=d[f+3];d[f]=h;d[f+1]=g;d[f+2]=k;d[f+3]=l;d=a[b];a[b]=a[c];a[c]=d}
 function hilbert(e,d){var a=e^d,b=65535^a,c=65535^(e|d),f=e&(d^65535),h=a>>1^a,g=c>>1^b&f>>1^c,k=a&c>>1^f>>1^f;a|=b>>1;b=h;c=g;f=k;h=a&b>>2^b&(a^b)>>2;g^=a&c>>2^b&f>>2;k^=b&c>>2^(a^b)&f>>2;a=a&a>>2^b&b>>2;b=h;c=g;f=k;h=a&b>>4^b&(a^b)>>4;g^=a&c>>4^b&f>>4;k^=b&c>>4^(a^b)&f>>4;a=a&a>>4^b&b>>4;b=h;c=g;f=k;g^=a&c>>8^b&f>>8;k^=b&c>>8^(a^b)&f>>8;a=e^d;g=k^k>>1|65535^(a|g^g>>1);a=(a|a<<8)&16711935;a=(a|a<<4)&252645135;a=(a|a<<2)&858993459;g=(g|g<<8)&16711935;g=(g|g<<4)&252645135;g=(g|g<<2)&858993459;return(((g|
 g<<1)&1431655765)<<1|(a|a<<1)&1431655765)>>>0};
