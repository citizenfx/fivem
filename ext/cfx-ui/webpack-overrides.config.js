const SentryWebpackPlugin = require("@sentry/webpack-plugin");
const webpack = require('webpack');

module.exports = (config, options) => {
	if (process.env.SENTRY_AUTH_TOKEN) {
		config.plugins.push(new webpack.EnvironmentPlugin(['CI_PIPELINE_ID']));

		config.plugins.push(new SentryWebpackPlugin({
			url: 'https://sentry.fivem.net/',
			authToken: process.env.SENTRY_AUTH_TOKEN,
			release: `cfx-${process.env.CI_PIPELINE_ID}`,
			org: "citizenfx",
			project: "cfx-ui",

			include: "./dist",
			ignore: ["node_modules", "webpack.config.js"],
		}));
	} else {
		config.plugins.push(new webpack.DefinePlugin({
			'process.env.CI_PIPELINE_ID': 'dev',
		}));
	}

	return config;
}
