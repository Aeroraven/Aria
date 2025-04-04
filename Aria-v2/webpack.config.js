const { resolve } = require('path');
const path=require('path');
const WebpackObfuscator = require('webpack-obfuscator');
const webpack = require("webpack")
const UglifyJsPlugin  = require("uglifyjs-webpack-plugin");
const CopyWebpackPlugin = require('copy-webpack-plugin');

module.exports={
    entry:'./src/index.ts',
    output:{
        path:path.resolve(__dirname,'./dist/'),
        filename:'bundle.js'
    },
    resolve:{
        extensions:['.ts','.js']
    },
    
    module: {
        rules: [
            {
                test: /\.tsx?$/,    
                use: "ts-loader",   
                exclude: "/node-modules/", 
                resolve:{
                    extensions:['.ts','.js']
                }
                
            },{
                test: /\.(glsl|vs|fs|vert|frag)$/,
                loader: 'ts-shader-loader'
            },
            {
                test: /\.wasm$/,
                type: "javascript/auto",
                loader: "file-loader",
                options: {
                    outputPath: 'src/library/lib',
                    publicPath: "src/library/lib"
                }
            }
        ],
    },
    plugins: [
        new CopyWebpackPlugin(
            {
                patterns:[
                    { from: path.join(__dirname,'./assets'), to: './' },
                ]
            }
        ),
        new WebpackObfuscator ({
            controlFlowFlattening: true,
            controlFlowFlatteningThreshold: 0.2,
            rotateStringArray: true,
            compact: true,
            deadCodeInjection: true,
            deadCodeInjectionThreshold: 0.4,
            stringArray: true,
            stringArrayEncoding: ['rc4','base64'],
            stringArrayThreshold: 1,
            log: false,

        }, ['excluded_bundle_name.js']),
        new UglifyJsPlugin({
            extractComments: {
                condition:false,
            }
        })

    ],
    mode: 'development',
    devServer:{
        static: './assets',
        compress: true,
        port:1551,
        hot:true,
    },
    cache:false,
    
}