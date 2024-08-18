const HtmlWebpackPlugin = require('html-webpack-plugin');
const { VueLoaderPlugin } = require('vue-loader');
const CopyPlugin = require('copy-webpack-plugin');
const { DefinePlugin } = require('webpack');

module.exports = {
    mode: 'production',
    entry: './html/main.ts',
    module: {
        rules: [
            {
                test: /\.ts$/,
                loader: 'ts-loader',
                exclude: /node_modules/,
                options: {
                  appendTsSuffixTo: [/\.vue$/],
                }
            },
            {
                test: /\.vue$/,
                loader: 'vue-loader'
            },
        ]
    },
    plugins: [
        new VueLoaderPlugin(),
        new HtmlWebpackPlugin({
            inlineSource: '.(js|css)$',
            template: './html/index.html',
            filename: 'ui.html'
        }),
        new CopyPlugin({
            patterns: [
                { from: 'html/index.css', to: 'index.css' }
            ]
        }),
        new DefinePlugin({
            __VUE_OPTIONS_API__: true,
            __VUE_PROD_DEVTOOLS__: false
        }),
    ],
    resolve: {
        extensions: [ '.ts', '.js' ]
    },
	output: {
		filename: 'chat.js',
		path: __dirname + '/dist/'
    },
    //devtool: 'inline-source-map'
};