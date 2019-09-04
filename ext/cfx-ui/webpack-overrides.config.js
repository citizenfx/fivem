const SvgStorePlugin = require('external-svg-sprite-loader');
const AngularCompilerPlugin = require('@ngtools/webpack/src');

module.exports = (config, options) => {
/*    
    config.plugins = [ new SvgStorePlugin(), ...config.plugins ];

    for (const r of config.module.rules) {
        if (r.test.toString() === /\.(eot|svg|cur|jpg|png|webp|gif|otf|ttf|woff|woff2|ani)$/.toString()) {
            r.test = /\.(eot|cur|jpg|png|webp|gif|otf|ttf|woff|woff2|ani)$/;
        } else if (r.test.toString() === /\.scss$|\.sass$/.toString() || r.test.toString() === /\.css$/.toString()) {
            for (let i = 0; i < r.use.length; i++) {
                if (r.use[i].loader === 'postcss-loader') {
                    r.use[i].options.plugins = (loader) => [
                        require('postcss-import')({ 
                            resolve: (url) => (url.startsWith('~') ? url.substr(1) : url),
                            load: (filename) => {
                                return new Promise((resolve, reject) => {
                                    loader.fs.readFile(filename, (err, data) => {
                                        if (err) {
                                            reject(err);
                            
                                            return;
                                        }
                        
                                        const content = data.toString();
                                        resolve(content);
                                    });
                                });
                            },
                        }),
                    ];

                    r.use[i].options.sourceMap = true;

                    r.use.splice(i, 0, {
                        loader: 'resolve-url-loader'
                    });

                    break;
                }
            }

            if (!process.env.WEBPACK_DEV_SERVER) {
                for (let i = 0; i < r.use.length; i++) {
                    if ((r.use[i].loader || r.use[i]).toString().match(/raw-loader|raw-css-loader/)) {
                        r.use.splice(i, 1, {
                            loader: 'css-loader'
                        });

                        break;
                    }
                }
            }

            for (let i = 0; i < r.use.length; i++) {
                if (r.use[i].loader === 'sass-loader') {
                    r.use[i].options.sourceMap = true;
                    r.use[i].options.sourceMapContents = false;
                }
            }

            console.log(r.use);
        }
    }

    config.module.rules = [
        {
            test: /\.svg$/,
            loader: SvgStorePlugin.loader
        },
        {
            test: /\.html$/,
            loader: 'raw-loader',
        },
        ...config.module.rules
    ];

    const index = config.plugins.findIndex(p => {
        return p instanceof AngularCompilerPlugin.AngularCompilerPlugin;
    });
    const oldOptions = config.plugins[index]._options;
    oldOptions.directTemplateLoading = false;
    config.plugins.splice(index);
    config.plugins.push(new AngularCompilerPlugin.AngularCompilerPlugin(oldOptions));
*/

    return config;
}