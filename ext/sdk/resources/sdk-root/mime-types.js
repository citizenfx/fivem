//
// mimetype.js - A catalog object of mime types based on file extensions
//
// @author: R. S. Doiel, <rsdoiel@gmail.com>
// copyright (c) 2012 all rights reserved
//
// Released under New the BSD License.
// See: http://opensource.org/licenses/bsd-license.php
//
/*jslint indent: 4 */
/*global require, exports */
(function (self) {
    "use strict";
	var path, MimeType;

	// If we're NodeJS I can use the path module.
	// If I'm MongoDB shell, not available.
	if (typeof require !== 'undefined') {
		path = require('path');
	} else {
		path = {
			extname: function (filename) {
				if (filename.lastIndexOf(".") > 0) {
					return filename.substr(filename.lastIndexOf("."));
				}
			}
		};
	}

	if (exports === undefined) {
		exports = {};
	}

	MimeType = {
		charset: 'UTF-8',
		catalog: {},
		lookup: function (fname, include_charset, default_mime_type) {
			var ext, charset = this.charset;

			if (include_charset === undefined) {
				include_charset = false;
			}

			if (typeof include_charset === "string") {
				charset = include_charset;
				include_charset = true;
			}

			if (path.extname !== undefined) {
				ext = path.extname(fname).toLowerCase();
			} else if (fname.lastIndexOf('.') > 0) {
				ext = fname.substr(fname.lastIndexOf('.')).toLowerCase();
			} else {
				ext = fname;
			}

			// Handle the special cases where their is no extension
			// e..g README, manifest, LICENSE, TODO
			if (ext === "") {
				ext = fname;
			}

			if (this.catalog[ext] !== undefined) {
				if (include_charset === true &&
                        this.catalog[ext].indexOf('text/') === 0 &&
                        this.catalog[ext].indexOf('charset') < 0) {
					return this.catalog[ext] + '; charset=' + charset;
				} else {
					return this.catalog[ext];
				}
			} else if (default_mime_type !== undefined) {
				if (include_charset === true &&
                        default_mime_type.indexOf('text/') === 0) {
					return default_mime_type + '; charset=' + charset;
				}
				return default_mime_type;
			}
			return false;
		},
		set: function (exts, mime_type_string) {
			var result = true, self = this;
            //console.log("DEBUG exts.indexOf(',')", typeof exts.indexOf(','), exts.indexOf(','));
			if (exts.indexOf(',') > -1) {
				exts.split(',').forEach(function (ext) {
					ext = ext.trim();
					self.catalog[ext] = mime_type_string;
					if (self.catalog[ext] !== mime_type_string) {
						result = false;
					}
				});
			} else {
				self.catalog[exts] = mime_type_string;
			}
			return result;
		},
		del: function (ext) {
			delete this.catalog[ext];
			return (this.catalog[ext] === undefined);
		},
		forEach: function (callback) {
			var self = this, ext;
			// Mongo 2.2. Shell doesn't support Object.keys()
			for (ext in self.catalog) {
				if (self.catalog.hasOwnProperty(ext)) {
					callback(ext, self.catalog[ext]);
				}
			}
			return self.catalog;
		}
	};

	// From Apache project's mime type list.
	MimeType.set(".ez", "application/andrew-inset");
	MimeType.set(".aw", "application/applixware");
	MimeType.set(".atom", "application/atom+xml");
	MimeType.set(".atomcat", "application/atomcat+xml");
	MimeType.set(".atomsvc", "application/atomsvc+xml");
	MimeType.set(".ccxml", "application/ccxml+xml");
	MimeType.set(".cu", "application/cu-seeme");
	MimeType.set(".davmount", "application/davmount+xml");
	MimeType.set(".ecma", "application/ecmascript");
	MimeType.set(".emma", "application/emma+xml");
	MimeType.set(".epub", "application/epub+zip");
	MimeType.set(".pfr", "application/font-tdpfr");
	MimeType.set(".stk", "application/hyperstudio");
	MimeType.set(".jar", "application/java-archive");
	MimeType.set(".ser", "application/java-serialized-object");
	MimeType.set(".class", "application/java-vm");
	MimeType.set(".js", "application/javascript");
	MimeType.set(".json", "application/json");
	MimeType.set(".lostxml", "application/lost+xml");
	MimeType.set(".hqx", "application/mac-binhex40");
	MimeType.set(".cpt", "application/mac-compactpro");
	MimeType.set(".mrc", "application/marc");
	MimeType.set(".ma,.nb,.mb", "application/mathematica");
	MimeType.set(".mathml", "application/mathml+xml");
	MimeType.set(".mbox", "application/mbox");
	MimeType.set(".mscml", "application/mediaservercontrol+xml");
	MimeType.set(".mp4s", "application/mp4");
	MimeType.set(".doc,.dot", "application/msword");
	MimeType.set(".mxf", "application/mxf");
	MimeType.set(".oda", "application/oda");
	MimeType.set(".opf", "application/oebps-package+xml");
	MimeType.set(".ogx", "application/ogg");
	MimeType.set(".onetoc,.onetoc2,.onetmp,.onepkg", "application/onenote");
	MimeType.set(".xer", "application/patch-ops-error+xml");
	MimeType.set(".pdf", "application/pdf");
	MimeType.set(".pgp", "application/pgp-encrypted");
	MimeType.set(".asc,.sig", "application/pgp-signature");
	MimeType.set(".prf", "application/pics-rules");
	MimeType.set(".p10", "application/pkcs10");
	MimeType.set(".p7m,.p7c", "application/pkcs7-mime");
	MimeType.set(".p7s", "application/pkcs7-signature");
	MimeType.set(".cer", "application/pkix-cert");
	MimeType.set(".crl", "application/pkix-crl");
	MimeType.set(".pkipath", "application/pkix-pkipath");
	MimeType.set(".pki", "application/pkixcmp");
	MimeType.set(".pls", "application/pls+xml");
	MimeType.set(".ai,.eps,.ps", "application/postscript");
	MimeType.set(".cww", "application/prs.cww");
	MimeType.set(".rdf", "application/rdf+xml");
	MimeType.set(".rif", "application/reginfo+xml");
	MimeType.set(".rnc", "application/relax-ng-compact-syntax");
	MimeType.set(".rl", "application/resource-lists+xml");
	MimeType.set(".rld", "application/resource-lists-diff+xml");
	MimeType.set(".rs", "application/rls-services+xml");
	MimeType.set(".rsd", "application/rsd+xml");
	MimeType.set(".rss", "application/rss+xml");
	MimeType.set(".rtf", "application/rtf");
	MimeType.set(".sbml", "application/sbml+xml");
	MimeType.set(".scq", "application/scvp-cv-request");
	MimeType.set(".scs", "application/scvp-cv-response");
	MimeType.set(".spq", "application/scvp-vp-request");
	MimeType.set(".spp", "application/scvp-vp-response");
	MimeType.set(".sdp", "application/sdp");
	MimeType.set(".setpay", "application/set-payment-initiation");
	MimeType.set(".setreg", "application/set-registration-initiation");
	MimeType.set(".shf", "application/shf+xml");
	MimeType.set(".smi,.smil", "application/smil+xml");
	MimeType.set(".rq", "application/sparql-query");
	MimeType.set(".srx", "application/sparql-results+xml");
	MimeType.set(".gram", "application/srgs");
	MimeType.set(".grxml", "application/srgs+xml");
	MimeType.set(".ssml", "application/ssml+xml");
	MimeType.set(".plb", "application/vnd.3gpp.pic-bw-large");
	MimeType.set(".psb", "application/vnd.3gpp.pic-bw-small");
	MimeType.set(".pvb", "application/vnd.3gpp.pic-bw-var");
	MimeType.set(".tcap", "application/vnd.3gpp2.tcap");
	MimeType.set(".pwn", "application/vnd.3m.post-it-notes");
	MimeType.set(".aso", "application/vnd.accpac.simply.aso");
	MimeType.set(".imp", "application/vnd.accpac.simply.imp");
	MimeType.set(".acu", "application/vnd.acucobol");
	MimeType.set(".atc,.acutc", "application/vnd.acucorp");
	MimeType.set(".air", "application/vnd.adobe.air-application-installer-package+zip");
	MimeType.set(".xdp", "application/vnd.adobe.xdp+xml");
	MimeType.set(".xfdf", "application/vnd.adobe.xfdf");
	MimeType.set(".azf", "application/vnd.airzip.filesecure.azf");
	MimeType.set(".azs", "application/vnd.airzip.filesecure.azs");
	MimeType.set(".azw", "application/vnd.amazon.ebook");
	MimeType.set(".acc", "application/vnd.americandynamics.acc");
	MimeType.set(".ami", "application/vnd.amiga.ami");
	MimeType.set(".apk", "application/vnd.android.package-archive");
	MimeType.set(".cii", "application/vnd.anser-web-certificate-issue-initiation");
	MimeType.set(".fti", "application/vnd.anser-web-funds-transfer-initiation");
	MimeType.set(".atx", "application/vnd.antix.game-component");
	MimeType.set(".mpkg", "application/vnd.apple.installer+xml");
	MimeType.set(".swi", "application/vnd.arastra.swi");
	MimeType.set(".aep", "application/vnd.audiograph");
	MimeType.set(".mpm", "application/vnd.blueice.multipass");
	MimeType.set(".bmi", "application/vnd.bmi");
	MimeType.set(".rep", "application/vnd.businessobjects");
	MimeType.set(".cdxml", "application/vnd.chemdraw+xml");
	MimeType.set(".mmd", "application/vnd.chipnuts.karaoke-mmd");
	MimeType.set(".cdy", "application/vnd.cinderella");
	MimeType.set(".cla", "application/vnd.claymore");
	MimeType.set(".c4g,.c4d,.c4f,.c4p,.c4u", "application/vnd.clonk.c4group");
	MimeType.set(".csp", "application/vnd.commonspace");
	MimeType.set(".cdbcmsg", "application/vnd.contact.cmsg");
	MimeType.set(".cmc", "application/vnd.cosmocaller");
	MimeType.set(".clkx", "application/vnd.crick.clicker");
	MimeType.set(".clkk", "application/vnd.crick.clicker.keyboard");
	MimeType.set(".clkp", "application/vnd.crick.clicker.palette");
	MimeType.set(".clkt", "application/vnd.crick.clicker.template");
	MimeType.set(".clkw", "application/vnd.crick.clicker.wordbank");
	MimeType.set(".wbs", "application/vnd.criticaltools.wbs+xml");
	MimeType.set(".pml", "application/vnd.ctc-posml");
	MimeType.set(".ppd", "application/vnd.cups-ppd");
	MimeType.set(".car", "application/vnd.curl.car");
	MimeType.set(".pcurl", "application/vnd.curl.pcurl");
	MimeType.set(".rdz", "application/vnd.data-vision.rdz");
	MimeType.set(".fe_launch", "application/vnd.denovo.fcselayout-link");
	MimeType.set(".dna", "application/vnd.dna");
	MimeType.set(".mlp", "application/vnd.dolby.mlp");
	MimeType.set(".dpg", "application/vnd.dpgraph");
	MimeType.set(".dfac", "application/vnd.dreamfactory");
	MimeType.set(".geo", "application/vnd.dynageo");
	MimeType.set(".mag", "application/vnd.ecowin.chart");
	MimeType.set(".nml", "application/vnd.enliven");
	MimeType.set(".esf", "application/vnd.epson.esf");
	MimeType.set(".msf", "application/vnd.epson.msf");
	MimeType.set(".qam", "application/vnd.epson.quickanime");
	MimeType.set(".slt", "application/vnd.epson.salt");
	MimeType.set(".ssf", "application/vnd.epson.ssf");
	MimeType.set(".es3,.et3", "application/vnd.eszigno3+xml");
	MimeType.set(".ez2", "application/vnd.ezpix-album");
	MimeType.set(".ez3", "application/vnd.ezpix-package");
	MimeType.set(".fdf", "application/vnd.fdf");
	MimeType.set(".mseed", "application/vnd.fdsn.mseed");
	MimeType.set(".seed,.dataless", "application/vnd.fdsn.seed");
	MimeType.set(".gph", "application/vnd.flographit");
	MimeType.set(".ftc", "application/vnd.fluxtime.clip");
	MimeType.set(".fm,.frame,.maker,.book", "application/vnd.framemaker");
	MimeType.set(".fnc", "application/vnd.frogans.fnc");
	MimeType.set(".ltf", "application/vnd.frogans.ltf");
	MimeType.set(".fsc", "application/vnd.fsc.weblaunch");
	MimeType.set(".oas", "application/vnd.fujitsu.oasys");
	MimeType.set(".oa2", "application/vnd.fujitsu.oasys2");
	MimeType.set(".oa3", "application/vnd.fujitsu.oasys3");
	MimeType.set(".fg5", "application/vnd.fujitsu.oasysgp");
	MimeType.set(".bh2", "application/vnd.fujitsu.oasysprs");
	MimeType.set(".ddd", "application/vnd.fujixerox.ddd");
	MimeType.set(".xdw", "application/vnd.fujixerox.docuworks");
	MimeType.set(".xbd", "application/vnd.fujixerox.docuworks.binder");
	MimeType.set(".fzs", "application/vnd.fuzzysheet");
	MimeType.set(".txd", "application/vnd.genomatix.tuxedo");
	MimeType.set(".ggb", "application/vnd.geogebra.file");
	MimeType.set(".ggt", "application/vnd.geogebra.tool");
	MimeType.set(".gex,.gre", "application/vnd.geometry-explorer");
	MimeType.set(".gmx", "application/vnd.gmx");
	MimeType.set(".kml", "application/vnd.google-earth.kml+xml");
	MimeType.set(".kmz", "application/vnd.google-earth.kmz");
	MimeType.set(".gqf,.gqs", "application/vnd.grafeq");
	MimeType.set(".gac", "application/vnd.groove-account");
	MimeType.set(".ghf", "application/vnd.groove-help");
	MimeType.set(".gim", "application/vnd.groove-identity-message");
	MimeType.set(".grv", "application/vnd.groove-injector");
	MimeType.set(".gtm", "application/vnd.groove-tool-message");
	MimeType.set(".tpl", "application/vnd.groove-tool-template");
	MimeType.set(".vcg", "application/vnd.groove-vcard");
	MimeType.set(".zmm", "application/vnd.handheld-entertainment+xml");
	MimeType.set(".hbci", "application/vnd.hbci");
	MimeType.set(".les", "application/vnd.hhe.lesson-player");
	MimeType.set(".hpgl", "application/vnd.hp-hpgl");
	MimeType.set(".hpid", "application/vnd.hp-hpid");
	MimeType.set(".hps", "application/vnd.hp-hps");
	MimeType.set(".jlt", "application/vnd.hp-jlyt");
	MimeType.set(".pcl", "application/vnd.hp-pcl");
	MimeType.set(".pclxl", "application/vnd.hp-pclxl");
	MimeType.set(".sfd-hdstx", "application/vnd.hydrostatix.sof-data");
	MimeType.set(".x3d", "application/vnd.hzn-3d-crossword");
	MimeType.set(".mpy", "application/vnd.ibm.minipay");
	MimeType.set(".afp,.listafp,.list3820", "application/vnd.ibm.modcap");
	MimeType.set(".irm", "application/vnd.ibm.rights-management");
	MimeType.set(".sc", "application/vnd.ibm.secure-container");
	MimeType.set(".icc,.icm", "application/vnd.iccprofile");
	MimeType.set(".igl", "application/vnd.igloader");
	MimeType.set(".ivp", "application/vnd.immervision-ivp");
	MimeType.set(".ivu", "application/vnd.immervision-ivu");
	MimeType.set(".xpw,.xpx", "application/vnd.intercon.formnet");
	MimeType.set(".qbo", "application/vnd.intu.qbo");
	MimeType.set(".qfx", "application/vnd.intu.qfx");
	MimeType.set(".rcprofile", "application/vnd.ipunplugged.rcprofile");
	MimeType.set(".irp", "application/vnd.irepository.package+xml");
	MimeType.set(".xpr", "application/vnd.is-xpr");
	MimeType.set(".jam", "application/vnd.jam");
	MimeType.set(".rms", "application/vnd.jcp.javame.midlet-rms");
	MimeType.set(".jisp", "application/vnd.jisp");
	MimeType.set(".joda", "application/vnd.joost.joda-archive");
	MimeType.set(".ktz,.ktr", "application/vnd.kahootz");
	MimeType.set(".karbon", "application/vnd.kde.karbon");
	MimeType.set(".chrt", "application/vnd.kde.kchart");
	MimeType.set(".kfo", "application/vnd.kde.kformula");
	MimeType.set(".flw", "application/vnd.kde.kivio");
	MimeType.set(".kon", "application/vnd.kde.kontour");
	MimeType.set(".kpr,.kpt", "application/vnd.kde.kpresenter");
	MimeType.set(".ksp", "application/vnd.kde.kspread");
	MimeType.set(".kwd,.kwt", "application/vnd.kde.kword");
	MimeType.set(".htke", "application/vnd.kenameaapp");
	MimeType.set(".kia", "application/vnd.kidspiration");
	MimeType.set(".kne,.knp", "application/vnd.kinar");
	MimeType.set(".skp,.skd,.skt,.skm", "application/vnd.koan");
	MimeType.set(".sse", "application/vnd.kodak-descriptor");
	MimeType.set(".lbd", "application/vnd.llamagraphics.life-balance.desktop");
	MimeType.set(".lbe", "application/vnd.llamagraphics.life-balance.exchange+xml");
	MimeType.set(".123", "application/vnd.lotus-1-2-3");
	MimeType.set(".apr", "application/vnd.lotus-approach");
	MimeType.set(".pre", "application/vnd.lotus-freelance");
	MimeType.set(".nsf", "application/vnd.lotus-notes");
	MimeType.set(".org", "application/vnd.lotus-organizer");
	MimeType.set(".scm", "application/vnd.lotus-screencam");
	MimeType.set(".lwp", "application/vnd.lotus-wordpro");
	MimeType.set(".portpkg", "application/vnd.macports.portpkg");
	MimeType.set(".mcd", "application/vnd.mcd");
	MimeType.set(".mc1", "application/vnd.medcalcdata");
	MimeType.set(".cdkey", "application/vnd.mediastation.cdkey");
	MimeType.set(".mwf", "application/vnd.mfer");
	MimeType.set(".mfm", "application/vnd.mfmp");
	MimeType.set(".flo", "application/vnd.micrografx.flo");
	MimeType.set(".igx", "application/vnd.micrografx.igx");
	MimeType.set(".mif", "application/vnd.mif");
	MimeType.set(".daf", "application/vnd.mobius.daf");
	MimeType.set(".dis", "application/vnd.mobius.dis");
	MimeType.set(".mbk", "application/vnd.mobius.mbk");
	MimeType.set(".mqy", "application/vnd.mobius.mqy");
	MimeType.set(".msl", "application/vnd.mobius.msl");
	MimeType.set(".plc", "application/vnd.mobius.plc");
	MimeType.set(".txf", "application/vnd.mobius.txf");
	MimeType.set(".mpn", "application/vnd.mophun.application");
	MimeType.set(".mpc", "application/vnd.mophun.certificate");
	MimeType.set(".xul", "application/vnd.mozilla.xul+xml");
	MimeType.set(".cil", "application/vnd.ms-artgalry");
	MimeType.set(".cab", "application/vnd.ms-cab-compressed");
	MimeType.set(".xls,.xlm,.xla,.xlc,.xlt,.xlw", "application/vnd.ms-excel");
	MimeType.set(".xlam", "application/vnd.ms-excel.addin.macroenabled.12");
	MimeType.set(".xlsb", "application/vnd.ms-excel.sheet.binary.macroenabled.12");
	MimeType.set(".xlsm", "application/vnd.ms-excel.sheet.macroenabled.12");
	MimeType.set(".xltm", "application/vnd.ms-excel.template.macroenabled.12");
	MimeType.set(".eot", "application/vnd.ms-fontobject");
	MimeType.set(".chm", "application/vnd.ms-htmlhelp");
	MimeType.set(".ims", "application/vnd.ms-ims");
	MimeType.set(".lrm", "application/vnd.ms-lrm");
	MimeType.set(".cat", "application/vnd.ms-pki.seccat");
	MimeType.set(".stl", "application/vnd.ms-pki.stl");
	MimeType.set(".ppt,.pps,.pot", "application/vnd.ms-powerpoint");
	MimeType.set(".ppam", "application/vnd.ms-powerpoint.addin.macroenabled.12");
	MimeType.set(".pptm", "application/vnd.ms-powerpoint.presentation.macroenabled.12");
	MimeType.set(".sldm", "application/vnd.ms-powerpoint.slide.macroenabled.12");
	MimeType.set(".ppsm", "application/vnd.ms-powerpoint.slideshow.macroenabled.12");
	MimeType.set(".potm", "application/vnd.ms-powerpoint.template.macroenabled.12");
	MimeType.set(".mpp,.mpt", "application/vnd.ms-project");
	MimeType.set(".docm", "application/vnd.ms-word.document.macroenabled.12");
	MimeType.set(".dotm", "application/vnd.ms-word.template.macroenabled.12");
	MimeType.set(".wps,.wks,.wcm,.wdb", "application/vnd.ms-works");
	MimeType.set(".wpl", "application/vnd.ms-wpl");
	MimeType.set(".xps", "application/vnd.ms-xpsdocument");
	MimeType.set(".mseq", "application/vnd.mseq");
	MimeType.set(".mus", "application/vnd.musician");
	MimeType.set(".msty", "application/vnd.muvee.style");
	MimeType.set(".nlu", "application/vnd.neurolanguage.nlu");
	MimeType.set(".nnd", "application/vnd.noblenet-directory");
	MimeType.set(".nns", "application/vnd.noblenet-sealer");
	MimeType.set(".nnw", "application/vnd.noblenet-web");
	MimeType.set(".ngdat", "application/vnd.nokia.n-gage.data");
	MimeType.set(".n-gage", "application/vnd.nokia.n-gage.symbian.install");
	MimeType.set(".rpst", "application/vnd.nokia.radio-preset");
	MimeType.set(".rpss", "application/vnd.nokia.radio-presets");
	MimeType.set(".edm", "application/vnd.novadigm.edm");
	MimeType.set(".edx", "application/vnd.novadigm.edx");
	MimeType.set(".ext", "application/vnd.novadigm.ext");
	MimeType.set(".odc", "application/vnd.oasis.opendocument.chart");
	MimeType.set(".otc", "application/vnd.oasis.opendocument.chart-template");
	MimeType.set(".odb", "application/vnd.oasis.opendocument.database");
	MimeType.set(".odf", "application/vnd.oasis.opendocument.formula");
	MimeType.set(".odft", "application/vnd.oasis.opendocument.formula-template");
	MimeType.set(".odg", "application/vnd.oasis.opendocument.graphics");
	MimeType.set(".otg", "application/vnd.oasis.opendocument.graphics-template");
	MimeType.set(".odi", "application/vnd.oasis.opendocument.image");
	MimeType.set(".oti", "application/vnd.oasis.opendocument.image-template");
	MimeType.set(".odp", "application/vnd.oasis.opendocument.presentation");
	MimeType.set(".ods", "application/vnd.oasis.opendocument.spreadsheet");
	MimeType.set(".ots", "application/vnd.oasis.opendocument.spreadsheet-template");
	MimeType.set(".odt", "application/vnd.oasis.opendocument.text");
	MimeType.set(".otm", "application/vnd.oasis.opendocument.text-master");
	MimeType.set(".ott", "application/vnd.oasis.opendocument.text-template");
	MimeType.set(".oth", "application/vnd.oasis.opendocument.text-web");
	MimeType.set(".xo", "application/vnd.olpc-sugar");
	MimeType.set(".dd2", "application/vnd.oma.dd2+xml");
	MimeType.set(".oxt", "application/vnd.openofficeorg.extension");
	MimeType.set(".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation");
	MimeType.set(".sldx", "application/vnd.openxmlformats-officedocument.presentationml.slide");
	MimeType.set(".ppsx", "application/vnd.openxmlformats-officedocument.presentationml.slideshow");
	MimeType.set(".potx", "application/vnd.openxmlformats-officedocument.presentationml.template");
	MimeType.set(".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
	MimeType.set(".xltx", "application/vnd.openxmlformats-officedocument.spreadsheetml.template");
	MimeType.set(".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
	MimeType.set(".dotx", "application/vnd.openxmlformats-officedocument.wordprocessingml.template");
	MimeType.set(".dp", "application/vnd.osgi.dp");
	MimeType.set(".pdb,.pqa,.oprc", "application/vnd.palm");
	MimeType.set(".str", "application/vnd.pg.format");
	MimeType.set(".ei6", "application/vnd.pg.osasli");
	MimeType.set(".efif", "application/vnd.picsel");
	MimeType.set(".plf", "application/vnd.pocketlearn");
	MimeType.set(".pbd", "application/vnd.powerbuilder6");
	MimeType.set(".box", "application/vnd.previewsystems.box");
	MimeType.set(".mgz", "application/vnd.proteus.magazine");
	MimeType.set(".qps", "application/vnd.publishare-delta-tree");
	MimeType.set(".ptid", "application/vnd.pvi.ptid1");
	MimeType.set(".qxd,.qxt,.qwd,.qwt,.qxl,.qxb", "application/vnd.quark.quarkxpress");
	MimeType.set(".mxl", "application/vnd.recordare.musicxml");
	MimeType.set(".musicxml", "application/vnd.recordare.musicxml+xml");
	MimeType.set(".cod", "application/vnd.rim.cod");
	MimeType.set(".rm", "application/vnd.rn-realmedia");
	MimeType.set(".link66", "application/vnd.route66.link66+xml");
	MimeType.set(".see", "application/vnd.seemail");
	MimeType.set(".sema", "application/vnd.sema");
	MimeType.set(".semd", "application/vnd.semd");
	MimeType.set(".semf", "application/vnd.semf");
	MimeType.set(".ifm", "application/vnd.shana.informed.formdata");
	MimeType.set(".itp", "application/vnd.shana.informed.formtemplate");
	MimeType.set(".iif", "application/vnd.shana.informed.interchange");
	MimeType.set(".ipk", "application/vnd.shana.informed.package");
	MimeType.set(".twd,.twds", "application/vnd.simtech-mindmapper");
	MimeType.set(".mmf", "application/vnd.smaf");
	MimeType.set(".teacher", "application/vnd.smart.teacher");
	MimeType.set(".sdkm,.sdkd", "application/vnd.solent.sdkm+xml");
	MimeType.set(".dxp", "application/vnd.spotfire.dxp");
	MimeType.set(".sfs", "application/vnd.spotfire.sfs");
	MimeType.set(".sdc", "application/vnd.stardivision.calc");
	MimeType.set(".sda", "application/vnd.stardivision.draw");
	MimeType.set(".sdd", "application/vnd.stardivision.impress");
	MimeType.set(".smf", "application/vnd.stardivision.math");
	MimeType.set(".sdw", "application/vnd.stardivision.writer");
	MimeType.set(".vor", "application/vnd.stardivision.writer");
	MimeType.set(".sgl", "application/vnd.stardivision.writer-global");
	MimeType.set(".sxc", "application/vnd.sun.xml.calc");
	MimeType.set(".stc", "application/vnd.sun.xml.calc.template");
	MimeType.set(".sxd", "application/vnd.sun.xml.draw");
	MimeType.set(".std", "application/vnd.sun.xml.draw.template");
	MimeType.set(".sxi", "application/vnd.sun.xml.impress");
	MimeType.set(".sti", "application/vnd.sun.xml.impress.template");
	MimeType.set(".sxm", "application/vnd.sun.xml.math");
	MimeType.set(".sxw", "application/vnd.sun.xml.writer");
	MimeType.set(".sxg", "application/vnd.sun.xml.writer.global");
	MimeType.set(".stw", "application/vnd.sun.xml.writer.template");
	MimeType.set(".sus,.susp", "application/vnd.sus-calendar");
	MimeType.set(".svd", "application/vnd.svd");
	MimeType.set(".sis,.sisx", "application/vnd.symbian.install");
	MimeType.set(".xsm", "application/vnd.syncml+xml");
	MimeType.set(".bdm", "application/vnd.syncml.dm+wbxml");
	MimeType.set(".xdm", "application/vnd.syncml.dm+xml");
	MimeType.set(".tao", "application/vnd.tao.intent-module-archive");
	MimeType.set(".tmo", "application/vnd.tmobile-livetv");
	MimeType.set(".tpt", "application/vnd.trid.tpt");
	MimeType.set(".mxs", "application/vnd.triscape.mxs");
	MimeType.set(".tra", "application/vnd.trueapp");
	MimeType.set(".ufd,.ufdl", "application/vnd.ufdl");
	MimeType.set(".utz", "application/vnd.uiq.theme");
	MimeType.set(".umj", "application/vnd.umajin");
	MimeType.set(".unityweb", "application/vnd.unity");
	MimeType.set(".uoml", "application/vnd.uoml+xml");
	MimeType.set(".vcx", "application/vnd.vcx");
	MimeType.set(".vsd,.vst,.vss,.vsw", "application/vnd.visio");
	MimeType.set(".vis", "application/vnd.visionary");
	MimeType.set(".vsf", "application/vnd.vsf");
	MimeType.set(".wbxml", "application/vnd.wap.wbxml");
	MimeType.set(".wmlc", "application/vnd.wap.wmlc");
	MimeType.set(".wmlsc", "application/vnd.wap.wmlscriptc");
	MimeType.set(".wtb", "application/vnd.webturbo");
	MimeType.set(".wpd", "application/vnd.wordperfect");
	MimeType.set(".wqd", "application/vnd.wqd");
	MimeType.set(".stf", "application/vnd.wt.stf");
	MimeType.set(".xar", "application/vnd.xara");
	MimeType.set(".xfdl", "application/vnd.xfdl");
	MimeType.set(".hvd", "application/vnd.yamaha.hv-dic");
	MimeType.set(".hvs", "application/vnd.yamaha.hv-script");
	MimeType.set(".hvp", "application/vnd.yamaha.hv-voice");
	MimeType.set(".osf", "application/vnd.yamaha.openscoreformat");
	MimeType.set(".osfpvg", "application/vnd.yamaha.openscoreformat.osfpvg+xml");
	MimeType.set(".saf", "application/vnd.yamaha.smaf-audio");
	MimeType.set(".spf", "application/vnd.yamaha.smaf-phrase");
	MimeType.set(".cmp", "application/vnd.yellowriver-custom-menu");
	MimeType.set(".zir,.zirz", "application/vnd.zul");
	MimeType.set(".zaz", "application/vnd.zzazz.deck+xml");
	MimeType.set(".vxml", "application/voicexml+xml");
	MimeType.set(".hlp", "application/winhlp");
	MimeType.set(".wsdl", "application/wsdl+xml");
	MimeType.set(".wspolicy", "application/wspolicy+xml");
	MimeType.set(".abw", "application/x-abiword");
	MimeType.set(".ace", "application/x-ace-compressed");
	MimeType.set(".aab,.x32,.u32,.vox", "application/x-authorware-bin");
	MimeType.set(".aam", "application/x-authorware-map");
	MimeType.set(".aas", "application/x-authorware-seg");
	MimeType.set(".bcpio", "application/x-bcpio");
	MimeType.set(".torrent", "application/x-bittorrent");
	MimeType.set(".bz", "application/x-bzip");
	MimeType.set(".bz2,.boz", "application/x-bzip2");
	MimeType.set(".vcd", "application/x-cdlink");
	MimeType.set(".chat", "application/x-chat");
	MimeType.set(".pgn", "application/x-chess-pgn");
	MimeType.set(".cpio", "application/x-cpio");
	MimeType.set(".csh", "application/x-csh");
	MimeType.set(".deb,.udeb", "application/x-debian-package");
	MimeType.set(".dir,.dcr,.dxr,.cst,.cct,.cxt,.w3d,.fgd,.swa", "application/x-director");
	MimeType.set(".wad", "application/x-doom");
	MimeType.set(".ncx", "application/x-dtbncx+xml");
	MimeType.set(".dtb", "application/x-dtbook+xml");
	MimeType.set(".res", "application/x-dtbresource+xml");
	MimeType.set(".dvi", "application/x-dvi");
	MimeType.set(".bdf", "application/x-font-bdf");
	MimeType.set(".gsf", "application/x-font-ghostscript");
	MimeType.set(".psf", "application/x-font-linux-psf");
	MimeType.set(".otf", "application/x-font-otf");
	MimeType.set(".pcf", "application/x-font-pcf");
	MimeType.set(".snf", "application/x-font-snf");
	MimeType.set(".ttf,.ttc", "application/x-font-ttf");
	MimeType.set(".woff", "application/font-woff");
	MimeType.set(".pfa,.pfb,.pfm,.afm", "application/x-font-type1");
	MimeType.set(".spl", "application/x-futuresplash");
	MimeType.set(".gnumeric", "application/x-gnumeric");
	MimeType.set(".gtar", "application/x-gtar");
	MimeType.set(".hdf", "application/x-hdf");
	MimeType.set(".jnlp", "application/x-java-jnlp-file");
	MimeType.set(".latex", "application/x-latex");
	MimeType.set(".prc,.mobi", "application/x-mobipocket-ebook");
	MimeType.set(".application", "application/x-ms-application");
	MimeType.set(".wmd", "application/x-ms-wmd");
	MimeType.set(".wmz", "application/x-ms-wmz");
	MimeType.set(".xbap", "application/x-ms-xbap");
	MimeType.set(".mdb", "application/x-msaccess");
	MimeType.set(".obd", "application/x-msbinder");
	MimeType.set(".crd", "application/x-mscardfile");
	MimeType.set(".clp", "application/x-msclip");
	MimeType.set(".exe,.dll,.com,.bat,.msi", "application/x-msdownload");
	MimeType.set(".mvb,.m13,.m14", "application/x-msmediaview");
	MimeType.set(".wmf", "application/x-msmetafile");
	MimeType.set(".mny", "application/x-msmoney");
	MimeType.set(".pub", "application/x-mspublisher");
	MimeType.set(".scd", "application/x-msschedule");
	MimeType.set(".trm", "application/x-msterminal");
	MimeType.set(".wri", "application/x-mswrite");
	MimeType.set(".nc,.cdf", "application/x-netcdf");
	MimeType.set(".p12,.pfx", "application/x-pkcs12");
	MimeType.set(".p7b,.spc", "application/x-pkcs7-certificates");
	MimeType.set(".p7r", "application/x-pkcs7-certreqresp");
	MimeType.set(".rar", "application/x-rar-compressed");
	MimeType.set(".sh", "application/x-sh");
	MimeType.set(".shar", "application/x-shar");
	MimeType.set(".swf", "application/x-shockwave-flash");
	MimeType.set(".xap", "application/x-silverlight-app");
	MimeType.set(".sit", "application/x-stuffit");
	MimeType.set(".sitx", "application/x-stuffitx");
	MimeType.set(".sv4cpio", "application/x-sv4cpio");
	MimeType.set(".sv4crc", "application/x-sv4crc");
	MimeType.set(".tar", "application/x-tar");
	MimeType.set(".tcl", "application/x-tcl");
	MimeType.set(".tex", "application/x-tex");
	MimeType.set(".tfm", "application/x-tex-tfm");
	MimeType.set(".texinfo,.texi", "application/x-texinfo");
	MimeType.set(".ustar", "application/x-ustar");
	MimeType.set(".src", "application/x-wais-source");
	MimeType.set(".der,.crt", "application/x-x509-ca-cert");
	MimeType.set(".fig", "application/x-xfig");
	MimeType.set(".xpi", "application/x-xpinstall");
	MimeType.set(".xenc", "application/xenc+xml");
	MimeType.set(".xhtml,.xht", "application/xhtml+xml");
	MimeType.set(".xml,.xsl", "application/xml");
	MimeType.set(".dtd", "application/xml-dtd");
	MimeType.set(".xop", "application/xop+xml");
	MimeType.set(".xslt", "application/xslt+xml");
	MimeType.set(".xspf", "application/xspf+xml");
	MimeType.set(".mxml,.xhvml,.xvml,.xvm", "application/xv+xml");
	MimeType.set(".zip", "application/zip");
	MimeType.set(".adp", "audio/adpcm");
	MimeType.set(".au,.snd", "audio/basic");
	MimeType.set(".mid,.midi,.kar,.rmi", "audio/midi");
	MimeType.set(".mp4a", "audio/mp4");
	MimeType.set(".m4a,.m4p", "audio/mp4a-latm");
	MimeType.set(".mpga,.mp2,.mp2a,.mp3,.m2a,.m3a", "audio/mpeg");
	MimeType.set(".oga,.ogg,.spx", "audio/ogg");
	MimeType.set(".eol", "audio/vnd.digital-winds");
	MimeType.set(".dts", "audio/vnd.dts");
	MimeType.set(".dtshd", "audio/vnd.dts.hd");
	MimeType.set(".lvp", "audio/vnd.lucent.voice");
	MimeType.set(".pya", "audio/vnd.ms-playready.media.pya");
	MimeType.set(".ecelp4800", "audio/vnd.nuera.ecelp4800");
	MimeType.set(".ecelp7470", "audio/vnd.nuera.ecelp7470");
	MimeType.set(".ecelp9600", "audio/vnd.nuera.ecelp9600");
	MimeType.set(".aac", "audio/x-aac");
	MimeType.set(".aif,.aiff,.aifc", "audio/x-aiff");
	MimeType.set(".m3u", "audio/x-mpegurl");
	MimeType.set(".wax", "audio/x-ms-wax");
	MimeType.set(".wma", "audio/x-ms-wma");
	MimeType.set(".ram,.ra", "audio/x-pn-realaudio");
	MimeType.set(".rmp", "audio/x-pn-realaudio-plugin");
	MimeType.set(".wav", "audio/x-wav");
	MimeType.set(".cdx", "chemical/x-cdx");
	MimeType.set(".cif", "chemical/x-cif");
	MimeType.set(".cmdf", "chemical/x-cmdf");
	MimeType.set(".cml", "chemical/x-cml");
	MimeType.set(".csml", "chemical/x-csml");
	MimeType.set(".xyz", "chemical/x-xyz");
	MimeType.set(".bmp", "image/bmp");
	MimeType.set(".cgm", "image/cgm");
	MimeType.set(".g3", "image/g3fax");
	MimeType.set(".gif", "image/gif");
	MimeType.set(".ief", "image/ief");
	MimeType.set(".jp2", "image/jp2");
	MimeType.set(".jpeg,.jpg,.jpe", "image/jpeg");
	MimeType.set(".pict,.pic,.pct", "image/pict");
	MimeType.set(".png", "image/png");
	MimeType.set(".btif", "image/prs.btif");
	MimeType.set(".svg,.svgz", "image/svg+xml");
	MimeType.set(".tiff,.tif", "image/tiff");
	MimeType.set(".psd", "image/vnd.adobe.photoshop");
	MimeType.set(".djvu,.djv", "image/vnd.djvu");
	MimeType.set(".dwg", "image/vnd.dwg");
	MimeType.set(".dxf", "image/vnd.dxf");
	MimeType.set(".fbs", "image/vnd.fastbidsheet");
	MimeType.set(".fpx", "image/vnd.fpx");
	MimeType.set(".fst", "image/vnd.fst");
	MimeType.set(".mmr", "image/vnd.fujixerox.edmics-mmr");
	MimeType.set(".rlc", "image/vnd.fujixerox.edmics-rlc");
	MimeType.set(".mdi", "image/vnd.ms-modi");
	MimeType.set(".npx", "image/vnd.net-fpx");
	MimeType.set(".wbmp", "image/vnd.wap.wbmp");
	MimeType.set(".xif", "image/vnd.xiff");
	MimeType.set(".ras", "image/x-cmu-raster");
	MimeType.set(".cmx", "image/x-cmx");
	MimeType.set(".fh,.fhc,.fh4,.fh5,.fh7", "image/x-freehand");
	MimeType.set(".ico", "image/x-icon");
	MimeType.set(".pntg,.pnt,.mac", "image/x-macpaint");
	MimeType.set(".pcx", "image/x-pcx");
	//MimeType.set(".pic,.pct", "image/x-pict");
	MimeType.set(".pnm", "image/x-portable-anymap");
	MimeType.set(".pbm", "image/x-portable-bitmap");
	MimeType.set(".pgm", "image/x-portable-graymap");
	MimeType.set(".ppm", "image/x-portable-pixmap");
	MimeType.set(".qtif,.qti", "image/x-quicktime");
	MimeType.set(".rgb", "image/x-rgb");
	MimeType.set(".xbm", "image/x-xbitmap");
	MimeType.set(".xpm", "image/x-xpixmap");
	MimeType.set(".xwd", "image/x-xwindowdump");
	MimeType.set(".eml,.mime", "message/rfc822");
	MimeType.set(".igs,.iges", "model/iges");
	MimeType.set(".msh,.mesh,.silo", "model/mesh");
	MimeType.set(".dwf", "model/vnd.dwf");
	MimeType.set(".gdl", "model/vnd.gdl");
	MimeType.set(".gtw", "model/vnd.gtw");
	MimeType.set(".mts", "model/vnd.mts");
	MimeType.set(".vtu", "model/vnd.vtu");
	MimeType.set(".wrl,.vrml", "model/vrml");
	MimeType.set(".ics,.ifb", "text/calendar");
	MimeType.set(".css", "text/css");
	MimeType.set(".csv", "text/csv");
	MimeType.set(".html,.htm", "text/html");
	MimeType.set(".txt,.text,.conf,.def,.list,.log,.in", "text/plain");
	MimeType.set(".dsc", "text/prs.lines.tag");
	MimeType.set(".rtx", "text/richtext");
	MimeType.set(".sgml,.sgm", "text/sgml");
	MimeType.set(".tsv", "text/tab-separated-values");
	MimeType.set(".t,.tr,.roff,.man,.me,.ms", "text/troff");
	MimeType.set(".uri,.uris,.urls", "text/uri-list");
	MimeType.set(".curl", "text/vnd.curl");
	MimeType.set(".dcurl", "text/vnd.curl.dcurl");
	MimeType.set(".scurl", "text/vnd.curl.scurl");
	MimeType.set(".mcurl", "text/vnd.curl.mcurl");
	MimeType.set(".fly", "text/vnd.fly");
	MimeType.set(".flx", "text/vnd.fmi.flexstor");
	MimeType.set(".gv", "text/vnd.graphviz");
	MimeType.set(".3dml", "text/vnd.in3d.3dml");
	MimeType.set(".spot", "text/vnd.in3d.spot");
	MimeType.set(".jad", "text/vnd.sun.j2me.app-descriptor");
	MimeType.set(".wml", "text/vnd.wap.wml");
	MimeType.set(".wmls", "text/vnd.wap.wmlscript");
	MimeType.set(".s,.asm", "text/x-asm");
	MimeType.set(".c,.cc,.cxx,.cpp,.h,.hh,.dic", "text/x-c");
	MimeType.set(".f,.for,.f77,.f90", "text/x-fortran");
	MimeType.set(".p,.pas", "text/x-pascal");
	MimeType.set(".java", "text/x-java-source");
	MimeType.set(".etx", "text/x-setext");
	MimeType.set(".uu", "text/x-uuencode");
	MimeType.set(".vcs", "text/x-vcalendar");
	MimeType.set(".vcf", "text/x-vcard");
	MimeType.set(".3gp", "video/3gpp");
	MimeType.set(".3g2", "video/3gpp2");
	MimeType.set(".h261", "video/h261");
	MimeType.set(".h263", "video/h263");
	MimeType.set(".h264", "video/h264");
	MimeType.set(".jpgv", "video/jpeg");
	MimeType.set(".jpm,.jpgm", "video/jpm");
	MimeType.set(".mj2,.mjp2", "video/mj2");
	MimeType.set(".mp4,.mp4v,.mpg4,.m4v", "video/mp4");
	MimeType.set(".mkv,.mk3d,.mka,.mks", "video/x-matroska");
	MimeType.set(".webm", "video/webm");
	MimeType.set(".mpeg,.mpg,.mpe,.m1v,.m2v", "video/mpeg");
	MimeType.set(".ogv", "video/ogg");
	MimeType.set(".qt,.mov", "video/quicktime");
	MimeType.set(".fvt", "video/vnd.fvt");
	MimeType.set(".mxu,.m4u", "video/vnd.mpegurl");
	MimeType.set(".pyv", "video/vnd.ms-playready.media.pyv");
	MimeType.set(".viv", "video/vnd.vivo");
	MimeType.set(".dv,.dif", "video/x-dv");
	MimeType.set(".f4v", "video/x-f4v");
	MimeType.set(".fli", "video/x-fli");
	MimeType.set(".flv", "video/x-flv");
	//MimeType.set(".m4v", "video/x-m4v");
	MimeType.set(".asf,.asx", "video/x-ms-asf");
	MimeType.set(".wm", "video/x-ms-wm");
	MimeType.set(".wmv", "video/x-ms-wmv");
	MimeType.set(".wmx", "video/x-ms-wmx");
	MimeType.set(".wvx", "video/x-ms-wvx");
	MimeType.set(".avi", "video/x-msvideo");
	MimeType.set(".movie", "video/x-sgi-movie");
	MimeType.set(".ice", "x-conference/x-cooltalk");
	MimeType.set(".indd", "application/x-indesign");
  MimeType.set(".dat", "application/octet-stream");

    // Compressed files
    // Based on notes at http://en.wikipedia.org/wiki/List_of_archive_formats
    MimeType.set(".gz", "application/x-gzip");
    MimeType.set(".tgz", "application/x-tar");
    MimeType.set(".tar", "application/x-tar");

	// Not really sure about these...
	MimeType.set(".epub", "application/epub+zip");
	MimeType.set(".mobi", "application/x-mobipocket-ebook");

	// Here's some common special cases without filename extensions
	MimeType.set("README,LICENSE,COPYING,TODO,ABOUT,AUTHORS,CONTRIBUTORS",
		"text/plain");
	MimeType.set("manifest,.manifest,.mf,.appcache", "text/cache-manifest");
	if (exports !== undefined) {
		exports.charset = MimeType.charset;
		exports.catalog = MimeType.catalog;
		exports.lookup = MimeType.lookup;
		exports.set = MimeType.set;
		exports.del = MimeType.del;
		exports.forEach = MimeType.forEach;
	}
    // Note: Chrome now defines window.MimeType, only define for legacy usage.
    if (self.MimeType === undefined) {
        self.MimeType = MimeType;
    }
    // Note: Per Hypercuded switch to camel case to avoid Chrome issues.
    if (self.mimeType === undefined) {
        self.mimeType = MimeType;
    }
	return self;
}(this));
