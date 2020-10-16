import { Component, OnInit, NgZone, Inject, ElementRef, ViewChild, AfterViewInit } from '@angular/core';

import { GameService } from './game.service';
import { TrackingService } from './tracking.service';

import { environment } from '../environments/environment';
import { Router, NavigationEnd } from '@angular/router';
import { interval } from 'rxjs';
import { startWith, map, distinctUntilChanged, filter, take, tap } from 'rxjs/operators';
import { ServersService } from './servers/servers.service';
import { L10N_LOCALE, L10nLocale, L10nTranslationService } from 'angular-l10n';
import { OverlayContainer } from '@angular/cdk/overlay';
import { getNavConfigFromUrl } from './nav/helpers';

// from fxdk
const vertexShaderSrc = `
  attribute vec2 a_position;
  attribute vec2 a_texcoord;
  uniform mat3 u_matrix;
  varying vec2 textureCoordinate;
  void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    textureCoordinate = a_texcoord;
  }
`;

const fragmentShaderSrc = `
varying highp vec2 textureCoordinate;
uniform sampler2D external_texture;
void main()
{
  gl_FragColor = texture2D(external_texture, textureCoordinate);
}
`;

function makeShader(gl: WebGLRenderingContext, type: number, src: string) {
	const shader = gl.createShader(type);

	gl.shaderSource(shader, src);
	gl.compileShader(shader);

	const infoLog = gl.getShaderInfoLog(shader);
	if (infoLog) {
		console.error(infoLog);
	}

	return shader;
}

function createTexture(gl: WebGLRenderingContext) {
	const tex = gl.createTexture();

	const texPixels = new Uint8Array([0, 0, 255, 255]);

	gl.bindTexture(gl.TEXTURE_2D, tex);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, texPixels);

	gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
	gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);

	// Magic hook sequence
	gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
	gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.MIRRORED_REPEAT);
	gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);

	// Reset
	gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

	return tex;
}

function createBuffers(gl: WebGLRenderingContext) {
	const vertexBuff = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuff);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
		-1, -1,
		1, -1,
		-1, 1,
		1, 1,
	]), gl.STATIC_DRAW);

	const texBuff = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, texBuff);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
		0, 0,
		1, 0,
		0, 1,
		1, 1,
	]), gl.STATIC_DRAW);

	return { vertexBuff, texBuff };
}

function createProgram(gl: WebGLRenderingContext) {
	const vertexShader = makeShader(gl, gl.VERTEX_SHADER, vertexShaderSrc);
	const fragmentShader = makeShader(gl, gl.FRAGMENT_SHADER, fragmentShaderSrc);

	const program = gl.createProgram();

	gl.attachShader(program, vertexShader);
	gl.attachShader(program, fragmentShader);
	gl.linkProgram(program);
	gl.useProgram(program);

	const vloc = gl.getAttribLocation(program, 'a_position');
	const tloc = gl.getAttribLocation(program, 'a_texcoord');

	return { program, vloc, tloc };
}

function createGameView(canvas: HTMLCanvasElement) {
	const gl = canvas.getContext('webgl', {
		antialias: false,
		depth: false,
		stencil: false,
		alpha: false,
		desynchronized: true,
		failIfMajorPerformanceCaveat: false
	}) as WebGLRenderingContext;

	let render = () => { };

	function createStuff() {
		const tex = createTexture(gl);
		const { program, vloc, tloc } = createProgram(gl);
		const { vertexBuff, texBuff } = createBuffers(gl);

		gl.useProgram(program);

		gl.bindTexture(gl.TEXTURE_2D, tex);

		gl.uniform1i(gl.getUniformLocation(program, 'external_texture'), 0);

		gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuff);
		gl.vertexAttribPointer(vloc, 2, gl.FLOAT, false, 0, 0);
		gl.enableVertexAttribArray(vloc);

		gl.bindBuffer(gl.ARRAY_BUFFER, texBuff);
		gl.vertexAttribPointer(tloc, 2, gl.FLOAT, false, 0, 0);
		gl.enableVertexAttribArray(tloc);

		gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);

		render();
	}

	const gameView = {
		canvas,
		gl,
		animationFrame: void 0,
		resize: (width: number, height: number) => {
			gl.viewport(0, 0, width, height);
			gl.canvas.width = width;
			gl.canvas.height = height;
		},
	};

	render = () => {
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
		gl.finish();

		gameView.animationFrame = requestAnimationFrame(render);
	};

	createStuff();

	return gameView;
}

@Component({
	selector: 'app-root',
	templateUrl: 'app.component.html',
	styleUrls: ['app.component.scss']
})
export class AppComponent implements OnInit, AfterViewInit {
	overlayActive = false;
	minModeSetUp = false;
	changelogShown = false;

	showSiteNavbar = !!environment.web;

	gameName = 'gta5';

	classes: { [key: string]: boolean } = {};

	@ViewChild('gameCanvas')
	gameCanvas: ElementRef;

	gameView: ReturnType<typeof createGameView>;

	get minMode() {
		return this.gameService.inMinMode;
	}

	get bgImage(): string {
		return this.minMode ? 'url(' + this.gameService.minmodeBlob['art:backgroundImage'] + ')' : '';
	}

	get splashScreen() {
		return this.router.url === '/';
	}

	constructor(@Inject(L10N_LOCALE) public locale: L10nLocale,
		public l10nService: L10nTranslationService,
		public gameService: GameService,
		private trackingService: TrackingService,
		private router: Router,
		private zone: NgZone,
		private serversService: ServersService,
		private overlayContainer: OverlayContainer,
	) {
		this.gameService.init();

		this.gameService.languageChange.subscribe(value => {
			this.l10nService.setLocale({ language: value });
		})

		this.gameService.minModeChanged.subscribe((value: boolean) => {
			if (value) {
				delete this.classes['theme-light'];
				this.classes['minmode'] = true;
				this.classes['theme-dark'] = true;
				this.classes = { ...this.classes };
				overlayContainer.getContainerElement().classList.add('theme-dark');
				this.router.navigate(['/minmode']);
			}

			this.minModeSetUp = true;
		});

		this.gameService.getConvar('ui_blurPerfMode').subscribe((value: string) => {
			delete this.classes['blur-noBackdrop'];
			delete this.classes['blur-noBlur'];

			if (value === 'none' || value === 'backdrop') {
				this.classes['blur-noBackdrop'] = true;
			}

			if (value === 'none') {
				this.classes['blur-noBlur'] = true;
			}

			// trigger change detection
			this.classes = {
				...this.classes
			};
		});

		this.gameName = gameService.gameName;

		router.events.subscribe(event => {
			const url = (<NavigationEnd>event).url;

			if (url) {
				const { withHomeButton } = getNavConfigFromUrl(url);

				this.classes['no-header-safe-zone'] = !withHomeButton;
				this.classes = { ...this.classes };
			}
		});

		let settled = false;
		const settle = () => {
			if (settled) {
				return;
			}

			settled = true;
			this.serversService.onInitialized();

			(<HTMLDivElement>document.querySelector('.booting')).style.opacity = '0';
			(<HTMLDivElement>document.querySelector('app-root')).style.opacity = '1';
		};

		if (environment.web || !environment.production) {
			setTimeout(() => {
				settle();
			}, 100);
			return;
		}

		// We will either show ui in 0.5s or earlier when it is ready
		// this way we can be sure we don't ever block ui with loader forever
		setTimeout(settle, 500);

		// reused snippet from https://dev.to/herodevs/route-fully-rendered-detection-in-angular-2nh4
		this.zone.runOutsideAngular(() => {
			// Check very regularly to see if the pending macrotasks have all cleared
			interval(10)
				.pipe(
					startWith(0), // So that we don't initially wait
					// Turn the interval number into the current state of the zone
					map(() => !this.zone.hasPendingMacrotasks),
					// Don't emit until the zone state actually flips from `false` to `true`
					distinctUntilChanged(),
					// Filter out unstable event. Only emit once the state is stable again
					filter(stateStable => stateStable === true),
					// Complete the observable after it emits the first result
					take(1),
					tap(settle)
				).subscribe();
		});
	}

	ngAfterViewInit(): void {
		this.gameView = createGameView(this.gameCanvas.nativeElement);
		this.gameView.resize(window.innerWidth, window.innerHeight);

		window.addEventListener('resize', () => {
			this.gameView.resize(window.innerWidth, window.innerHeight);
		});
	}

	ngOnInit() {
		const themeName = this.gameService.gameName === 'rdr3'
			? 'theme-rdr3'
			: (this.gameService.darkTheme ? 'theme-dark' : 'theme-light');

		this.classes = {};
		this.classes[environment.web ? 'webapp' : 'gameapp'] = true;
		this.classes[themeName] = true;

		this.overlayContainer.getContainerElement().classList.add(themeName);

		this.classes['game-' + this.gameService.gameName] = true;
		this.classes['theRoot'] = true;
		this.classes['no-header-safe-zone'] = !getNavConfigFromUrl(this.router.url).withHomeButton;

		this.gameService.darkThemeChange.subscribe(value => {
			if (this.gameService.gameName !== 'rdr3') {
				const overlayElement = this.overlayContainer.getContainerElement();

				overlayElement.classList.remove('theme-light');
				overlayElement.classList.remove('theme-dark');

				delete this.classes['theme-light'];
				delete this.classes['theme-dark'];

				const themeName = value
					? 'theme-dark'
					: 'theme-light';

				this.classes[themeName] = true;
				overlayElement.classList.add(themeName);

				this.classes = {
					...this.classes
				};
			}
		});

		const lang = this.gameService.language;
		if (lang && this.l10nService.getAvailableLanguages().includes(lang)) {
			this.l10nService.setLocale({ language: lang });
		}
	}
}
