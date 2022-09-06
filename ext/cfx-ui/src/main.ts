import { enableProdMode, ApplicationRef } from '@angular/core';
import { platformBrowserDynamic } from '@angular/platform-browser-dynamic';
import * as Sentry from '@sentry/angular';
import { Integrations } from '@sentry/tracing';

import { AppModule } from './app/app.module';
import { environment } from './environments/environment';
import { enableDebugTools } from '@angular/platform-browser';

Object.defineProperty(window, 'parent', {
  get: () => window
});

import './spatial-navigation-polyfill.js';

if (process.env.CI_PIPELINE_ID !== 'dev') {
	Sentry.init({
		dsn: 'https://7670be5cc3a84ab5a617c5cd0121d0a0@sentry.fivem.net/9',
		integrations: [
			new Integrations.BrowserTracing({
				tracingOrigins: ['localhost', /\.fivem.net/, /\.cfx.re/],
				routingInstrumentation: Sentry.routingInstrumentation,
			}),
		],
		release: `cfx-${process.env.CI_PIPELINE_ID}`,

		tracesSampleRate: 0.05,
	});
}

if (environment.production) {
  enableProdMode();
}

platformBrowserDynamic().bootstrapModule(AppModule).then(moduleRef => {
  const appRef = moduleRef.injector.get(ApplicationRef);
  const componentRef = appRef.components[0];
  enableDebugTools(componentRef);
});
