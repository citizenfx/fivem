const CopyPlugin = require("copy-webpack-plugin");


module.exports = {
    mode: 'production',
    entry: './code.jsx',
    output: {
        filename: 'code.js'
    },
    module: {
        rules: [
            {
                test: /\.(jsx|js)$/,
                include: __dirname,
                exclude: /node_modules/,
                use: [{
                    loader: 'babel-loader',
                    options: {
                        presets: [
                            ['@babel/preset-env', {
                                "targets": "defaults"
                            }],
                            '@babel/preset-react'
                        ]
                    }
                }]
            },
            {
                test: /\.scss$/,
                use: [
                    'style-loader',
                    'css-loader',
                    'sass-loader'
                ],
            },                    
        ]
    },
    plugins: [
        new CopyPlugin({
            patterns: [
                { from: "*.html", to: "" },
                { from: "*.min.js", to: "" },
            ],
        }),
    ],
};