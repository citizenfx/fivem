/*
	https://github.com/kawanet/bufferish

	The MIT License (MIT)

	Copyright (c) 2016 Yusuke Kawasaki

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

! function(t, r) {
	"object" == typeof exports && "object" == typeof module ? module.exports = r() : "function" == typeof define && define.amd ? define([], r) : "object" == typeof exports ? exports.Bufferish = r() : t.Bufferish = r()

    if (typeof Buffer === 'undefined') {
        t.Buffer = t.Bufferish;
    }
}(this, function() {
	return function(t) {
		function r(n) {
			if (e[n]) return e[n].exports;
			var i = e[n] = {
				exports: {},
				id: n,
				loaded: !1
			};
			return t[n].call(i.exports, i, i.exports, r), i.loaded = !0, i.exports
		}
		var e = {};
		return r.m = t, r.c = e, r.p = "", r(0)
	}([function(t, r, e) {
		t.exports = e(1)
	}, function(t, r, e) {
		function n(t) {
			return t instanceof ArrayBuffer || c(t)
		}

		function i(t) {
			return null != t && n(t.buffer)
		}

		function o() {
			return !1
		}

		function u(t) {
			return t = "[object " + t + "]",
				function(r) {
					return null != r && {}.toString.call(r) === t
				}
		}
		var a = r.original = e(2),
			f = !(!a || !a.isBuffer),
			s = "undefined" != typeof ArrayBuffer;
		r.isArray = Array.isArray || u("Array"), r.isArrayBuffer = s ? n : o, r.isBuffer = f ? a.isBuffer : o, r.isView = s ? ArrayBuffer.isView || i : o;
		var h = e(7);
		r.alloc = h.alloc, r.concat = h.concat, r.from = h.from, r.prototype = e(8), r.Array = e(10), f && (r.Buffer = e(11)), s && (r.Uint8Array = e(12));
		var c = u("ArrayBuffer")
	}, function(t, r, e) {
		(function(r) {
			function e(t) {
				return t && t.isBuffer && t
			}
			t.exports = e("undefined" != typeof r && r) || e(this.Buffer) || e("undefined" != typeof window && window.Buffer) || this.Buffer
		}).call(r, e(3).Buffer)
	}, function(t, r, e) {
		(function(t, n) {
			/*!

				 * The buffer module from node.js, for the browser.

				 *

				 * @author   Feross Aboukhadijeh <feross@feross.org> <http://feross.org>

				 * @license  MIT

				 */

			"use strict";

			function i() {
				function t() {}
				try {
					var r = new Uint8Array(1);
					return r.foo = function() {
						return 42
					}, r.constructor = t, 42 === r.foo() && r.constructor === t && "function" == typeof r.subarray && 0 === r.subarray(1, 1).byteLength
				} catch (e) {
					return !1
				}
			}

			function o() {
				return t.TYPED_ARRAY_SUPPORT ? 2147483647 : 1073741823
			}

			function t(r) {
				return this instanceof t ? (t.TYPED_ARRAY_SUPPORT || (this.length = 0, this.parent = void 0), "number" == typeof r ? u(this, r) : "string" == typeof r ? a(this, r, arguments.length > 1 ? arguments[1] : "utf8") : f(this, r)) : arguments.length > 1 ? new t(r, arguments[1]) : new t(r)
			}

			function u(r, e) {
				if (r = g(r, e < 0 ? 0 : 0 | w(e)), !t.TYPED_ARRAY_SUPPORT)
					for (var n = 0; n < e; n++) r[n] = 0;
				return r
			}

			function a(t, r, e) {
				"string" == typeof e && "" !== e || (e = "utf8");
				var n = 0 | E(r, e);
				return t = g(t, n), t.write(r, e), t
			}

			function f(r, e) {
				if (t.isBuffer(e)) return s(r, e);
				if (K(e)) return h(r, e);
				if (null == e) throw new TypeError("must start with number, buffer, array or string");
				if ("undefined" != typeof ArrayBuffer) {
					if (e.buffer instanceof ArrayBuffer) return c(r, e);
					if (e instanceof ArrayBuffer) return l(r, e)
				}
				return e.length ? p(r, e) : y(r, e)
			}

			function s(t, r) {
				var e = 0 | w(r.length);
				return t = g(t, e), r.copy(t, 0, 0, e), t
			}

			function h(t, r) {
				var e = 0 | w(r.length);
				t = g(t, e);
				for (var n = 0; n < e; n += 1) t[n] = 255 & r[n];
				return t
			}

			function c(t, r) {
				var e = 0 | w(r.length);
				t = g(t, e);
				for (var n = 0; n < e; n += 1) t[n] = 255 & r[n];
				return t
			}

			function l(r, e) {
				return t.TYPED_ARRAY_SUPPORT ? (e.byteLength, r = t._augment(new Uint8Array(e))) : r = c(r, new Uint8Array(e)), r
			}

			function p(t, r) {
				var e = 0 | w(r.length);
				t = g(t, e);
				for (var n = 0; n < e; n += 1) t[n] = 255 & r[n];
				return t
			}

			function y(t, r) {
				var e, n = 0;
				"Buffer" === r.type && K(r.data) && (e = r.data, n = 0 | w(e.length)), t = g(t, n);
				for (var i = 0; i < n; i += 1) t[i] = 255 & e[i];
				return t
			}

			function g(r, e) {
				t.TYPED_ARRAY_SUPPORT ? (r = t._augment(new Uint8Array(e)), r.__proto__ = t.prototype) : (r.length = e, r._isBuffer = !0);
				var n = 0 !== e && e <= t.poolSize >>> 1;
				return n && (r.parent = Q), r
			}

			function w(t) {
				if (t >= o()) throw new RangeError("Attempt to allocate Buffer larger than maximum size: 0x" + o().toString(16) + " bytes");
				return 0 | t
			}

			function d(r, e) {
				if (!(this instanceof d)) return new d(r, e);
				var n = new t(r, e);
				return delete n.parent, n
			}

			function E(t, r) {
				"string" != typeof t && (t = "" + t);
				var e = t.length;
				if (0 === e) return 0;
				for (var n = !1;;) switch (r) {
					case "ascii":
					case "binary":
					case "raw":
					case "raws":
						return e;
					case "utf8":
					case "utf-8":
						return z(t).length;
					case "ucs2":
					case "ucs-2":
					case "utf16le":
					case "utf-16le":
						return 2 * e;
					case "hex":
						return e >>> 1;
					case "base64":
						return X(t).length;
					default:
						if (n) return z(t).length;
						r = ("" + r).toLowerCase(), n = !0
				}
			}

			function A(t, r, e) {
				var n = !1;
				if (r = 0 | r, e = void 0 === e || e === 1 / 0 ? this.length : 0 | e, t || (t = "utf8"), r < 0 && (r = 0), e > this.length && (e = this.length), e <= r) return "";
				for (;;) switch (t) {
					case "hex":
						return S(this, r, e);
					case "utf8":
					case "utf-8":
						return _(this, r, e);
					case "ascii":
						return T(this, r, e);
					case "binary":
						return L(this, r, e);
					case "base64":
						return R(this, r, e);
					case "ucs2":
					case "ucs-2":
					case "utf16le":
					case "utf-16le":
						return Y(this, r, e);
					default:
						if (n) throw new TypeError("Unknown encoding: " + t);
						t = (t + "").toLowerCase(), n = !0
				}
			}

			function v(t, r, e, n) {
				e = Number(e) || 0;
				var i = t.length - e;
				n ? (n = Number(n), n > i && (n = i)) : n = i;
				var o = r.length;
				if (o % 2 !== 0) throw new Error("Invalid hex string");
				n > o / 2 && (n = o / 2);
				for (var u = 0; u < n; u++) {
					var a = parseInt(r.substr(2 * u, 2), 16);
					if (isNaN(a)) throw new Error("Invalid hex string");
					t[e + u] = a
				}
				return u
			}

			function B(t, r, e, n) {
				return Z(z(r, t.length - e), t, e, n)
			}

			function b(t, r, e, n) {
				return Z(q(r), t, e, n)
			}

			function I(t, r, e, n) {
				return b(t, r, e, n)
			}

			function U(t, r, e, n) {
				return Z(X(r), t, e, n)
			}

			function m(t, r, e, n) {
				return Z(J(r, t.length - e), t, e, n)
			}

			function R(t, r, e) {
				return 0 === r && e === t.length ? G.fromByteArray(t) : G.fromByteArray(t.slice(r, e))
			}

			function _(t, r, e) {
				e = Math.min(t.length, e);
				for (var n = [], i = r; i < e;) {
					var o = t[i],
						u = null,
						a = o > 239 ? 4 : o > 223 ? 3 : o > 191 ? 2 : 1;
					if (i + a <= e) {
						var f, s, h, c;
						switch (a) {
							case 1:
								o < 128 && (u = o);
								break;
							case 2:
								f = t[i + 1], 128 === (192 & f) && (c = (31 & o) << 6 | 63 & f, c > 127 && (u = c));
								break;
							case 3:
								f = t[i + 1], s = t[i + 2], 128 === (192 & f) && 128 === (192 & s) && (c = (15 & o) << 12 | (63 & f) << 6 | 63 & s, c > 2047 && (c < 55296 || c > 57343) && (u = c));
								break;
							case 4:
								f = t[i + 1], s = t[i + 2], h = t[i + 3], 128 === (192 & f) && 128 === (192 & s) && 128 === (192 & h) && (c = (15 & o) << 18 | (63 & f) << 12 | (63 & s) << 6 | 63 & h, c > 65535 && c < 1114112 && (u = c))
						}
					}
					null === u ? (u = 65533, a = 1) : u > 65535 && (u -= 65536, n.push(u >>> 10 & 1023 | 55296), u = 56320 | 1023 & u), n.push(u), i += a
				}
				return P(n)
			}

			function P(t) {
				var r = t.length;
				if (r <= W) return String.fromCharCode.apply(String, t);
				for (var e = "", n = 0; n < r;) e += String.fromCharCode.apply(String, t.slice(n, n += W));
				return e
			}

			function T(t, r, e) {
				var n = "";
				e = Math.min(t.length, e);
				for (var i = r; i < e; i++) n += String.fromCharCode(127 & t[i]);
				return n
			}

			function L(t, r, e) {
				var n = "";
				e = Math.min(t.length, e);
				for (var i = r; i < e; i++) n += String.fromCharCode(t[i]);
				return n
			}

			function S(t, r, e) {
				var n = t.length;
				(!r || r < 0) && (r = 0), (!e || e < 0 || e > n) && (e = n);
				for (var i = "", o = r; o < e; o++) i += j(t[o]);
				return i
			}

			function Y(t, r, e) {
				for (var n = t.slice(r, e), i = "", o = 0; o < n.length; o += 2) i += String.fromCharCode(n[o] + 256 * n[o + 1]);
				return i
			}

			function x(t, r, e) {
				if (t % 1 !== 0 || t < 0) throw new RangeError("offset is not uint");
				if (t + r > e) throw new RangeError("Trying to access beyond buffer length")
			}

			function D(r, e, n, i, o, u) {
				if (!t.isBuffer(r)) throw new TypeError("buffer must be a Buffer instance");
				if (e > o || e < u) throw new RangeError("value is out of bounds");
				if (n + i > r.length) throw new RangeError("index out of range")
			}

			function O(t, r, e, n) {
				r < 0 && (r = 65535 + r + 1);
				for (var i = 0, o = Math.min(t.length - e, 2); i < o; i++) t[e + i] = (r & 255 << 8 * (n ? i : 1 - i)) >>> 8 * (n ? i : 1 - i)
			}

			function C(t, r, e, n) {
				r < 0 && (r = 4294967295 + r + 1);
				for (var i = 0, o = Math.min(t.length - e, 4); i < o; i++) t[e + i] = r >>> 8 * (n ? i : 3 - i) & 255
			}

			function M(t, r, e, n, i, o) {
				if (r > i || r < o) throw new RangeError("value is out of bounds");
				if (e + n > t.length) throw new RangeError("index out of range");
				if (e < 0) throw new RangeError("index out of range")
			}

			function N(t, r, e, n, i) {
				return i || M(t, r, e, 4, 3.4028234663852886e38, -3.4028234663852886e38), H.write(t, r, e, n, 23, 4), e + 4
			}

			function F(t, r, e, n, i) {
				return i || M(t, r, e, 8, 1.7976931348623157e308, -1.7976931348623157e308), H.write(t, r, e, n, 52, 8), e + 8
			}

			function k(t) {
				if (t = V(t).replace(tt, ""), t.length < 2) return "";
				for (; t.length % 4 !== 0;) t += "=";
				return t
			}

			function V(t) {
				return t.trim ? t.trim() : t.replace(/^\s+|\s+$/g, "")
			}

			function j(t) {
				return t < 16 ? "0" + t.toString(16) : t.toString(16)
			}

			function z(t, r) {
				r = r || 1 / 0;
				for (var e, n = t.length, i = null, o = [], u = 0; u < n; u++) {
					if (e = t.charCodeAt(u), e > 55295 && e < 57344) {
						if (!i) {
							if (e > 56319) {
								(r -= 3) > -1 && o.push(239, 191, 189);
								continue
							}
							if (u + 1 === n) {
								(r -= 3) > -1 && o.push(239, 191, 189);
								continue
							}
							i = e;
							continue
						}
						if (e < 56320) {
							(r -= 3) > -1 && o.push(239, 191, 189), i = e;
							continue
						}
						e = (i - 55296 << 10 | e - 56320) + 65536
					} else i && (r -= 3) > -1 && o.push(239, 191, 189);
					if (i = null, e < 128) {
						if ((r -= 1) < 0) break;
						o.push(e)
					} else if (e < 2048) {
						if ((r -= 2) < 0) break;
						o.push(e >> 6 | 192, 63 & e | 128)
					} else if (e < 65536) {
						if ((r -= 3) < 0) break;
						o.push(e >> 12 | 224, e >> 6 & 63 | 128, 63 & e | 128)
					} else {
						if (!(e < 1114112)) throw new Error("Invalid code point");
						if ((r -= 4) < 0) break;
						o.push(e >> 18 | 240, e >> 12 & 63 | 128, e >> 6 & 63 | 128, 63 & e | 128)
					}
				}
				return o
			}

			function q(t) {
				for (var r = [], e = 0; e < t.length; e++) r.push(255 & t.charCodeAt(e));
				return r
			}

			function J(t, r) {
				for (var e, n, i, o = [], u = 0; u < t.length && !((r -= 2) < 0); u++) e = t.charCodeAt(u), n = e >> 8, i = e % 256, o.push(i), o.push(n);
				return o
			}

			function X(t) {
				return G.toByteArray(k(t))
			}

			function Z(t, r, e, n) {
				for (var i = 0; i < n && !(i + e >= r.length || i >= t.length); i++) r[i + e] = t[i];
				return i
			}
			var G = e(4),
				H = e(5),
				K = e(6);
			r.Buffer = t, r.SlowBuffer = d, r.INSPECT_MAX_BYTES = 50, t.poolSize = 8192;
			var Q = {};
			t.TYPED_ARRAY_SUPPORT = void 0 !== n.TYPED_ARRAY_SUPPORT ? n.TYPED_ARRAY_SUPPORT : i(), t.TYPED_ARRAY_SUPPORT ? (t.prototype.__proto__ = Uint8Array.prototype, t.__proto__ = Uint8Array) : (t.prototype.length = void 0, t.prototype.parent = void 0), t.isBuffer = function(t) {
				return !(null == t || !t._isBuffer)
			}, t.compare = function(r, e) {
				if (!t.isBuffer(r) || !t.isBuffer(e)) throw new TypeError("Arguments must be Buffers");
				if (r === e) return 0;
				for (var n = r.length, i = e.length, o = 0, u = Math.min(n, i); o < u && r[o] === e[o];) ++o;
				return o !== u && (n = r[o], i = e[o]), n < i ? -1 : i < n ? 1 : 0
			}, t.isEncoding = function(t) {
				switch (String(t).toLowerCase()) {
					case "hex":
					case "utf8":
					case "utf-8":
					case "ascii":
					case "binary":
					case "base64":
					case "raw":
					case "ucs2":
					case "ucs-2":
					case "utf16le":
					case "utf-16le":
						return !0;
					default:
						return !1
				}
			}, t.concat = function(r, e) {
				if (!K(r)) throw new TypeError("list argument must be an Array of Buffers.");
				if (0 === r.length) return new t(0);
				var n;
				if (void 0 === e)
					for (e = 0, n = 0; n < r.length; n++) e += r[n].length;
				var i = new t(e),
					o = 0;
				for (n = 0; n < r.length; n++) {
					var u = r[n];
					u.copy(i, o), o += u.length
				}
				return i
			}, t.byteLength = E, t.prototype.toString = function() {
				var t = 0 | this.length;
				return 0 === t ? "" : 0 === arguments.length ? _(this, 0, t) : A.apply(this, arguments)
			}, t.prototype.equals = function(r) {
				if (!t.isBuffer(r)) throw new TypeError("Argument must be a Buffer");
				return this === r || 0 === t.compare(this, r)
			}, t.prototype.inspect = function() {
				var t = "",
					e = r.INSPECT_MAX_BYTES;
				return this.length > 0 && (t = this.toString("hex", 0, e).match(/.{2}/g).join(" "), this.length > e && (t += " ... ")), "<Buffer " + t + ">"
			}, t.prototype.compare = function(r) {
				if (!t.isBuffer(r)) throw new TypeError("Argument must be a Buffer");
				return this === r ? 0 : t.compare(this, r)
			}, t.prototype.indexOf = function(r, e) {
				function n(t, r, e) {
					for (var n = -1, i = 0; e + i < t.length; i++)
						if (t[e + i] === r[n === -1 ? 0 : i - n]) {
							if (n === -1 && (n = i), i - n + 1 === r.length) return e + n
						} else n = -1;
					return -1
				}
				if (e > 2147483647 ? e = 2147483647 : e < -2147483648 && (e = -2147483648), e >>= 0, 0 === this.length) return -1;
				if (e >= this.length) return -1;
				if (e < 0 && (e = Math.max(this.length + e, 0)), "string" == typeof r) return 0 === r.length ? -1 : String.prototype.indexOf.call(this, r, e);
				if (t.isBuffer(r)) return n(this, r, e);
				if ("number" == typeof r) return t.TYPED_ARRAY_SUPPORT && "function" === Uint8Array.prototype.indexOf ? Uint8Array.prototype.indexOf.call(this, r, e) : n(this, [r], e);
				throw new TypeError("val must be string, number or Buffer")
			}, t.prototype.get = function(t) {
				return console.log(".get() is deprecated. Access using array indexes instead."), this.readUInt8(t)
			}, t.prototype.set = function(t, r) {
				return console.log(".set() is deprecated. Access using array indexes instead."), this.writeUInt8(t, r)
			}, t.prototype.write = function(t, r, e, n) {
				if (void 0 === r) n = "utf8", e = this.length, r = 0;
				else if (void 0 === e && "string" == typeof r) n = r, e = this.length, r = 0;
				else if (isFinite(r)) r = 0 | r, isFinite(e) ? (e = 0 | e, void 0 === n && (n = "utf8")) : (n = e, e = void 0);
				else {
					var i = n;
					n = r, r = 0 | e, e = i
				}
				var o = this.length - r;
				if ((void 0 === e || e > o) && (e = o), t.length > 0 && (e < 0 || r < 0) || r > this.length) throw new RangeError("attempt to write outside buffer bounds");
				n || (n = "utf8");
				for (var u = !1;;) switch (n) {
					case "hex":
						return v(this, t, r, e);
					case "utf8":
					case "utf-8":
						return B(this, t, r, e);
					case "ascii":
						return b(this, t, r, e);
					case "binary":
						return I(this, t, r, e);
					case "base64":
						return U(this, t, r, e);
					case "ucs2":
					case "ucs-2":
					case "utf16le":
					case "utf-16le":
						return m(this, t, r, e);
					default:
						if (u) throw new TypeError("Unknown encoding: " + n);
						n = ("" + n).toLowerCase(), u = !0
				}
			}, t.prototype.toJSON = function() {
				return {
					type: "Buffer",
					data: Array.prototype.slice.call(this._arr || this, 0)
				}
			};
			var W = 4096;
			t.prototype.slice = function(r, e) {
				var n = this.length;
				r = ~~r, e = void 0 === e ? n : ~~e, r < 0 ? (r += n, r < 0 && (r = 0)) : r > n && (r = n), e < 0 ? (e += n, e < 0 && (e = 0)) : e > n && (e = n), e < r && (e = r);
				var i;
				if (t.TYPED_ARRAY_SUPPORT) i = t._augment(this.subarray(r, e));
				else {
					var o = e - r;
					i = new t(o, (void 0));
					for (var u = 0; u < o; u++) i[u] = this[u + r]
				}
				return i.length && (i.parent = this.parent || this), i
			}, t.prototype.readUIntLE = function(t, r, e) {
				t = 0 | t, r = 0 | r, e || x(t, r, this.length);
				for (var n = this[t], i = 1, o = 0; ++o < r && (i *= 256);) n += this[t + o] * i;
				return n
			}, t.prototype.readUIntBE = function(t, r, e) {
				t = 0 | t, r = 0 | r, e || x(t, r, this.length);
				for (var n = this[t + --r], i = 1; r > 0 && (i *= 256);) n += this[t + --r] * i;
				return n
			}, t.prototype.readUInt8 = function(t, r) {
				return r || x(t, 1, this.length), this[t]
			}, t.prototype.readUInt16LE = function(t, r) {
				return r || x(t, 2, this.length), this[t] | this[t + 1] << 8
			}, t.prototype.readUInt16BE = function(t, r) {
				return r || x(t, 2, this.length), this[t] << 8 | this[t + 1]
			}, t.prototype.readUInt32LE = function(t, r) {
				return r || x(t, 4, this.length), (this[t] | this[t + 1] << 8 | this[t + 2] << 16) + 16777216 * this[t + 3]
			}, t.prototype.readUInt32BE = function(t, r) {
				return r || x(t, 4, this.length), 16777216 * this[t] + (this[t + 1] << 16 | this[t + 2] << 8 | this[t + 3])
			}, t.prototype.readIntLE = function(t, r, e) {
				t = 0 | t, r = 0 | r, e || x(t, r, this.length);
				for (var n = this[t], i = 1, o = 0; ++o < r && (i *= 256);) n += this[t + o] * i;
				return i *= 128, n >= i && (n -= Math.pow(2, 8 * r)), n
			}, t.prototype.readIntBE = function(t, r, e) {
				t = 0 | t, r = 0 | r, e || x(t, r, this.length);
				for (var n = r, i = 1, o = this[t + --n]; n > 0 && (i *= 256);) o += this[t + --n] * i;
				return i *= 128, o >= i && (o -= Math.pow(2, 8 * r)), o
			}, t.prototype.readInt8 = function(t, r) {
				return r || x(t, 1, this.length), 128 & this[t] ? (255 - this[t] + 1) * -1 : this[t]
			}, t.prototype.readInt16LE = function(t, r) {
				r || x(t, 2, this.length);
				var e = this[t] | this[t + 1] << 8;
				return 32768 & e ? 4294901760 | e : e
			}, t.prototype.readInt16BE = function(t, r) {
				r || x(t, 2, this.length);
				var e = this[t + 1] | this[t] << 8;
				return 32768 & e ? 4294901760 | e : e
			}, t.prototype.readInt32LE = function(t, r) {
				return r || x(t, 4, this.length), this[t] | this[t + 1] << 8 | this[t + 2] << 16 | this[t + 3] << 24
			}, t.prototype.readInt32BE = function(t, r) {
				return r || x(t, 4, this.length), this[t] << 24 | this[t + 1] << 16 | this[t + 2] << 8 | this[t + 3]
			}, t.prototype.readFloatLE = function(t, r) {
				return r || x(t, 4, this.length), H.read(this, t, !0, 23, 4)
			}, t.prototype.readFloatBE = function(t, r) {
				return r || x(t, 4, this.length), H.read(this, t, !1, 23, 4)
			}, t.prototype.readDoubleLE = function(t, r) {
				return r || x(t, 8, this.length), H.read(this, t, !0, 52, 8)
			}, t.prototype.readDoubleBE = function(t, r) {
				return r || x(t, 8, this.length), H.read(this, t, !1, 52, 8)
			}, t.prototype.writeUIntLE = function(t, r, e, n) {
				t = +t, r = 0 | r, e = 0 | e, n || D(this, t, r, e, Math.pow(2, 8 * e), 0);
				var i = 1,
					o = 0;
				for (this[r] = 255 & t; ++o < e && (i *= 256);) this[r + o] = t / i & 255;
				return r + e
			}, t.prototype.writeUIntBE = function(t, r, e, n) {
				t = +t, r = 0 | r, e = 0 | e, n || D(this, t, r, e, Math.pow(2, 8 * e), 0);
				var i = e - 1,
					o = 1;
				for (this[r + i] = 255 & t; --i >= 0 && (o *= 256);) this[r + i] = t / o & 255;
				return r + e
			}, t.prototype.writeUInt8 = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 1, 255, 0), t.TYPED_ARRAY_SUPPORT || (r = Math.floor(r)), this[e] = 255 & r, e + 1
			}, t.prototype.writeUInt16LE = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 2, 65535, 0), t.TYPED_ARRAY_SUPPORT ? (this[e] = 255 & r, this[e + 1] = r >>> 8) : O(this, r, e, !0), e + 2
			}, t.prototype.writeUInt16BE = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 2, 65535, 0), t.TYPED_ARRAY_SUPPORT ? (this[e] = r >>> 8, this[e + 1] = 255 & r) : O(this, r, e, !1), e + 2
			}, t.prototype.writeUInt32LE = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 4, 4294967295, 0), t.TYPED_ARRAY_SUPPORT ? (this[e + 3] = r >>> 24, this[e + 2] = r >>> 16, this[e + 1] = r >>> 8, this[e] = 255 & r) : C(this, r, e, !0), e + 4
			}, t.prototype.writeUInt32BE = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 4, 4294967295, 0), t.TYPED_ARRAY_SUPPORT ? (this[e] = r >>> 24, this[e + 1] = r >>> 16, this[e + 2] = r >>> 8, this[e + 3] = 255 & r) : C(this, r, e, !1), e + 4
			}, t.prototype.writeIntLE = function(t, r, e, n) {
				if (t = +t, r = 0 | r, !n) {
					var i = Math.pow(2, 8 * e - 1);
					D(this, t, r, e, i - 1, -i)
				}
				var o = 0,
					u = 1,
					a = t < 0 ? 1 : 0;
				for (this[r] = 255 & t; ++o < e && (u *= 256);) this[r + o] = (t / u >> 0) - a & 255;
				return r + e
			}, t.prototype.writeIntBE = function(t, r, e, n) {
				if (t = +t, r = 0 | r, !n) {
					var i = Math.pow(2, 8 * e - 1);
					D(this, t, r, e, i - 1, -i)
				}
				var o = e - 1,
					u = 1,
					a = t < 0 ? 1 : 0;
				for (this[r + o] = 255 & t; --o >= 0 && (u *= 256);) this[r + o] = (t / u >> 0) - a & 255;
				return r + e
			}, t.prototype.writeInt8 = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 1, 127, -128), t.TYPED_ARRAY_SUPPORT || (r = Math.floor(r)), r < 0 && (r = 255 + r + 1), this[e] = 255 & r, e + 1
			}, t.prototype.writeInt16LE = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 2, 32767, -32768), t.TYPED_ARRAY_SUPPORT ? (this[e] = 255 & r, this[e + 1] = r >>> 8) : O(this, r, e, !0), e + 2
			}, t.prototype.writeInt16BE = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 2, 32767, -32768), t.TYPED_ARRAY_SUPPORT ? (this[e] = r >>> 8, this[e + 1] = 255 & r) : O(this, r, e, !1), e + 2
			}, t.prototype.writeInt32LE = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 4, 2147483647, -2147483648), t.TYPED_ARRAY_SUPPORT ? (this[e] = 255 & r, this[e + 1] = r >>> 8, this[e + 2] = r >>> 16, this[e + 3] = r >>> 24) : C(this, r, e, !0), e + 4
			}, t.prototype.writeInt32BE = function(r, e, n) {
				return r = +r, e = 0 | e, n || D(this, r, e, 4, 2147483647, -2147483648), r < 0 && (r = 4294967295 + r + 1), t.TYPED_ARRAY_SUPPORT ? (this[e] = r >>> 24, this[e + 1] = r >>> 16, this[e + 2] = r >>> 8, this[e + 3] = 255 & r) : C(this, r, e, !1), e + 4
			}, t.prototype.writeFloatLE = function(t, r, e) {
				return N(this, t, r, !0, e)
			}, t.prototype.writeFloatBE = function(t, r, e) {
				return N(this, t, r, !1, e)
			}, t.prototype.writeDoubleLE = function(t, r, e) {
				return F(this, t, r, !0, e)
			}, t.prototype.writeDoubleBE = function(t, r, e) {
				return F(this, t, r, !1, e)
			}, t.prototype.copy = function(r, e, n, i) {
				if (n || (n = 0), i || 0 === i || (i = this.length), e >= r.length && (e = r.length), e || (e = 0), i > 0 && i < n && (i = n), i === n) return 0;
				if (0 === r.length || 0 === this.length) return 0;
				if (e < 0) throw new RangeError("targetStart out of bounds");
				if (n < 0 || n >= this.length) throw new RangeError("sourceStart out of bounds");
				if (i < 0) throw new RangeError("sourceEnd out of bounds");
				i > this.length && (i = this.length), r.length - e < i - n && (i = r.length - e + n);
				var o, u = i - n;
				if (this === r && n < e && e < i)
					for (o = u - 1; o >= 0; o--) r[o + e] = this[o + n];
				else if (u < 1e3 || !t.TYPED_ARRAY_SUPPORT)
					for (o = 0; o < u; o++) r[o + e] = this[o + n];
				else r._set(this.subarray(n, n + u), e);
				return u
			}, t.prototype.fill = function(t, r, e) {
				if (t || (t = 0), r || (r = 0), e || (e = this.length), e < r) throw new RangeError("end < start");
				if (e !== r && 0 !== this.length) {
					if (r < 0 || r >= this.length) throw new RangeError("start out of bounds");
					if (e < 0 || e > this.length) throw new RangeError("end out of bounds");
					var n;
					if ("number" == typeof t)
						for (n = r; n < e; n++) this[n] = t;
					else {
						var i = z(t.toString()),
							o = i.length;
						for (n = r; n < e; n++) this[n] = i[n % o]
					}
					return this
				}
			}, t.prototype.toArrayBuffer = function() {
				if ("undefined" != typeof Uint8Array) {
					if (t.TYPED_ARRAY_SUPPORT) return new t(this).buffer;
					for (var r = new Uint8Array(this.length), e = 0, n = r.length; e < n; e += 1) r[e] = this[e];
					return r.buffer
				}
				throw new TypeError("Buffer.toArrayBuffer not supported in this browser")
			};
			var $ = t.prototype;
			t._augment = function(r) {
				return r.constructor = t, r._isBuffer = !0, r._set = r.set, r.get = $.get, r.set = $.set, r.write = $.write, r.toString = $.toString, r.toLocaleString = $.toString, r.toJSON = $.toJSON, r.equals = $.equals, r.compare = $.compare, r.indexOf = $.indexOf, r.copy = $.copy, r.slice = $.slice, r.readUIntLE = $.readUIntLE, r.readUIntBE = $.readUIntBE, r.readUInt8 = $.readUInt8, r.readUInt16LE = $.readUInt16LE, r.readUInt16BE = $.readUInt16BE, r.readUInt32LE = $.readUInt32LE, r.readUInt32BE = $.readUInt32BE, r.readIntLE = $.readIntLE, r.readIntBE = $.readIntBE, r.readInt8 = $.readInt8, r.readInt16LE = $.readInt16LE, r.readInt16BE = $.readInt16BE, r.readInt32LE = $.readInt32LE, r.readInt32BE = $.readInt32BE, r.readFloatLE = $.readFloatLE, r.readFloatBE = $.readFloatBE, r.readDoubleLE = $.readDoubleLE, r.readDoubleBE = $.readDoubleBE, r.writeUInt8 = $.writeUInt8, r.writeUIntLE = $.writeUIntLE, r.writeUIntBE = $.writeUIntBE, r.writeUInt16LE = $.writeUInt16LE, r.writeUInt16BE = $.writeUInt16BE, r.writeUInt32LE = $.writeUInt32LE, r.writeUInt32BE = $.writeUInt32BE, r.writeIntLE = $.writeIntLE, r.writeIntBE = $.writeIntBE, r.writeInt8 = $.writeInt8, r.writeInt16LE = $.writeInt16LE, r.writeInt16BE = $.writeInt16BE, r.writeInt32LE = $.writeInt32LE, r.writeInt32BE = $.writeInt32BE, r.writeFloatLE = $.writeFloatLE, r.writeFloatBE = $.writeFloatBE, r.writeDoubleLE = $.writeDoubleLE, r.writeDoubleBE = $.writeDoubleBE, r.fill = $.fill, r.inspect = $.inspect, r.toArrayBuffer = $.toArrayBuffer, r
			};
			var tt = /[^+\/0-9A-Za-z-_]/g
		}).call(r, e(3).Buffer, function() {
			return this
		}())
	}, function(t, r, e) {
		var n = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		! function(t) {
			"use strict";

			function r(t) {
				var r = t.charCodeAt(0);
				return r === u || r === c ? 62 : r === a || r === l ? 63 : r < f ? -1 : r < f + 10 ? r - f + 26 + 26 : r < h + 26 ? r - h : r < s + 26 ? r - s + 26 : void 0
			}

			function e(t) {
				function e(t) {
					s[c++] = t
				}
				var n, i, u, a, f, s;
				if (t.length % 4 > 0) throw new Error("Invalid string. Length must be a multiple of 4");
				var h = t.length;
				f = "=" === t.charAt(h - 2) ? 2 : "=" === t.charAt(h - 1) ? 1 : 0, s = new o(3 * t.length / 4 - f), u = f > 0 ? t.length - 4 : t.length;
				var c = 0;
				for (n = 0, i = 0; n < u; n += 4, i += 3) a = r(t.charAt(n)) << 18 | r(t.charAt(n + 1)) << 12 | r(t.charAt(n + 2)) << 6 | r(t.charAt(n + 3)), e((16711680 & a) >> 16), e((65280 & a) >> 8), e(255 & a);
				return 2 === f ? (a = r(t.charAt(n)) << 2 | r(t.charAt(n + 1)) >> 4, e(255 & a)) : 1 === f && (a = r(t.charAt(n)) << 10 | r(t.charAt(n + 1)) << 4 | r(t.charAt(n + 2)) >> 2, e(a >> 8 & 255), e(255 & a)), s
			}

			function i(t) {
				function r(t) {
					return n.charAt(t)
				}

				function e(t) {
					return r(t >> 18 & 63) + r(t >> 12 & 63) + r(t >> 6 & 63) + r(63 & t)
				}
				var i, o, u, a = t.length % 3,
					f = "";
				for (i = 0, u = t.length - a; i < u; i += 3) o = (t[i] << 16) + (t[i + 1] << 8) + t[i + 2], f += e(o);
				switch (a) {
					case 1:
						o = t[t.length - 1], f += r(o >> 2), f += r(o << 4 & 63), f += "==";
						break;
					case 2:
						o = (t[t.length - 2] << 8) + t[t.length - 1], f += r(o >> 10), f += r(o >> 4 & 63), f += r(o << 2 & 63), f += "="
				}
				return f
			}
			var o = "undefined" != typeof Uint8Array ? Uint8Array : Array,
				u = "+".charCodeAt(0),
				a = "/".charCodeAt(0),
				f = "0".charCodeAt(0),
				s = "a".charCodeAt(0),
				h = "A".charCodeAt(0),
				c = "-".charCodeAt(0),
				l = "_".charCodeAt(0);
			t.toByteArray = e, t.fromByteArray = i
		}(r)
	}, function(t, r) {
		r.read = function(t, r, e, n, i) {
			var o, u, a = 8 * i - n - 1,
				f = (1 << a) - 1,
				s = f >> 1,
				h = -7,
				c = e ? i - 1 : 0,
				l = e ? -1 : 1,
				p = t[r + c];
			for (c += l, o = p & (1 << -h) - 1, p >>= -h, h += a; h > 0; o = 256 * o + t[r + c], c += l, h -= 8);
			for (u = o & (1 << -h) - 1, o >>= -h, h += n; h > 0; u = 256 * u + t[r + c], c += l, h -= 8);
			if (0 === o) o = 1 - s;
			else {
				if (o === f) return u ? NaN : (p ? -1 : 1) * (1 / 0);
				u += Math.pow(2, n), o -= s
			}
			return (p ? -1 : 1) * u * Math.pow(2, o - n)
		}, r.write = function(t, r, e, n, i, o) {
			var u, a, f, s = 8 * o - i - 1,
				h = (1 << s) - 1,
				c = h >> 1,
				l = 23 === i ? Math.pow(2, -24) - Math.pow(2, -77) : 0,
				p = n ? 0 : o - 1,
				y = n ? 1 : -1,
				g = r < 0 || 0 === r && 1 / r < 0 ? 1 : 0;
			for (r = Math.abs(r), isNaN(r) || r === 1 / 0 ? (a = isNaN(r) ? 1 : 0, u = h) : (u = Math.floor(Math.log(r) / Math.LN2), r * (f = Math.pow(2, -u)) < 1 && (u--, f *= 2), r += u + c >= 1 ? l / f : l * Math.pow(2, 1 - c), r * f >= 2 && (u++, f /= 2), u + c >= h ? (a = 0, u = h) : u + c >= 1 ? (a = (r * f - 1) * Math.pow(2, i), u += c) : (a = r * Math.pow(2, c - 1) * Math.pow(2, i), u = 0)); i >= 8; t[e + p] = 255 & a, p += y, a /= 256, i -= 8);
			for (u = u << i | a, s += i; s > 0; t[e + p] = 255 & u, p += y, u /= 256, s -= 8);
			t[e + p - y] |= 128 * g
		}
	}, function(t, r) {
		var e = {}.toString;
		t.exports = Array.isArray || function(t) {
			return "[object Array]" == e.call(t)
		}
	}, function(t, r, e) {
		function n(t) {
			return "string" == typeof t ? u.call(this, t) : a(this).from(t)
		}

		function i(t) {
			return a(this).alloc(t)
		}

		function o(t, e) {
			function n(t) {
				e += t.length
			}

			function o(t) {
				s += f.prototype.copy.call(t, a, s)
			}
			e || (e = 0, Array.prototype.forEach.call(t, n));
			var u = this !== r && this || t[0],
				a = i.call(u, e),
				s = 0;
			return Array.prototype.forEach.call(t, o), a
		}

		function u(t) {
			var r = 3 * t.length,
				e = i.call(this, r),
				n = f.prototype.write.call(e, t);
			return r !== n && (e = f.prototype.slice.call(e, 0, n)), e
		}

		function a(t) {
			return f.isBuffer(t) ? f.Buffer : f.isView(t) ? f.Uint8Array : f.isArray(t) ? f.Array : f.Buffer || f.Uint8Array || f.Array
		}
		var f = e(1);
		r.alloc = i, r.concat = o, r.from = n
	}, function(t, r, e) {
		function n(t, r, e, n) {
			var o = s.isBuffer(this),
				u = s.isBuffer(t);
			if (o && u) return this.copy.apply(this, arguments);
			if (h || o || u || !s.isView(this) || !s.isView(t)) return f.copy.apply(this, arguments);
			var a = e || null != n ? i.call(this, e, n) : this;
			return t.set(a, r), a.length
		}

		function i(t, r) {
			t |= 0, r = null != r ? 0 | r : this.length;
			var e = this.slice || !h && this.subarray || f.slice;
			return e.call(this, t, r)
		}

		function o(t, r, e) {
			var n = s.isBuffer(this) ? this.toString : f.toString;
			return n.apply(this, arguments)
		}

		function u(t) {
			function r() {
				var r = this[t] || f[t];
				return r.apply(this, arguments)
			}
			return r
		}

		function a() {
			var t = new Uint8Array(1);
			return t.subarray && 0 === t.subarray(1, 1).byteLength
		}
		var f = e(9);
		r.copy = n, r.slice = i, r.toString = o, r.write = u("write");
		var s = e(1),
			h = s.hasArrayBuffer && !a()
	}, function(t, r, e) {
		function n(t, r) {
			for (var e = this, n = r || (r |= 0), i = t.length, o = 0; o < i; o++) {
				var u = t.charCodeAt(o);
				u < 128 ? e[n++] = u : u < 2048 ? (e[n++] = 192 | u >> 6, e[n++] = 128 | 63 & u) : (e[n++] = 224 | u >> 12, e[n++] = 128 | u >> 6 & 63, e[n++] = 128 | 63 & u)
			}
			return n - r
		}

		function i(t, r, e) {
			var n, i, o = this;
			r |= 0, e = 0 | e || o.length;
			var u = e - r;
			for (u > a && (u = a); r < e;) {
				n || null == i || (n = [i]);
				for (var f = new Array(u), s = 0; s < u && r < e;) {
					var h = o[r++];
					h = h < 128 ? h : h < 224 ? (63 & h) << 6 | 63 & o[r++] : (63 & h) << 12 | (63 & o[r++]) << 6 | 63 & o[r++], f[s++] = h
				}
				s < u && (f = f.slice(0, s)), i = String.fromCharCode.apply("", f), n && n.push(i)
			}
			return n ? n.join("") : i || ""
		}

		function o(t, r, e, n) {
			var i;
			e || (e = 0), n || 0 === n || (n = this.length), r || (r = 0);
			var o = n - e;
			if (t === this && e < r && r < n)
				for (i = o - 1; i >= 0; i--) t[i + r] = this[i + e];
			else
				for (i = 0; i < o; i++) t[i + r] = this[i + e];
			return o
		}

		function u(t, r) {
			var e = f.alloc.call(this, r - t);
			return f.copy.call(this, e, 0, t, r), e
		}
		var a = 8192;
		r.copy = o, r.slice = u, r.toString = i, r.write = n;
		var f = e(1)
	}, function(t, r, e) {
		function n(t) {
			return new Array(t)
		}

		function i(t) {
			if (!o.isBuffer(t) && o.isView(t)) t = o.Uint8Array.from(t);
			else if (o.isArrayBuffer(t)) t = new Uint8Array(t);
			else {
				if ("string" == typeof t) return o.from.call(r, t);
				if ("number" == typeof t) throw new TypeError('"value" argument must not be a number')
			}
			return Array.prototype.slice.call(t)
		}
		var o = e(1),
			r = t.exports = n(0);
		r.alloc = n, r.concat = o.concat, r.from = i
	}, function(t, r, e) {
		function n(t) {
			return new u(t)
		}

		function i(t) {
			if (!o.isBuffer(t) && o.isView(t)) t = o.Uint8Array.from(t);
			else if (o.isArrayBuffer(t)) t = new Uint8Array(t);
			else {
				if ("string" == typeof t) return o.from.call(r, t);
				if ("number" == typeof t) throw new TypeError('"value" argument must not be a number')
			}
			return u.from && 1 !== u.from.length ? u.from(t) : new u(t)
		}
		var o = e(1),
			u = o.original,
			r = t.exports = n(0);
		r.alloc = u.alloc || n, r.concat = o.concat, r.from = i
	}, function(t, r, e) {
		function n(t) {
			return new Uint8Array(t)
		}

		function i(t) {
			if (o.isView(t)) {
				var e = t.byteOffset,
					n = t.byteLength;
				t = t.buffer, t.byteLength !== n && (t.slice ? t = t.slice(e, e + n) : (t = new Uint8Array(t), t.byteLength !== n && (t = Array.prototype.slice.call(t, e, e + n))))
			} else {
				if ("string" == typeof t) return o.from.call(r, t);
				if ("number" == typeof t) throw new TypeError('"value" argument must not be a number')
			}
			return new Uint8Array(t)
		}
		var o = e(1),
			r = t.exports = n(0);
		r.alloc = n, r.concat = o.concat, r.from = i
	}])
});