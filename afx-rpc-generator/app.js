const fs = require('fs');
const path = require('path');
const merge = require('deepmerge');
const generator = require('@open-rpc/generator');

const args = process.argv;

var document_name = null;

var compile_files = [];
var compile_lang = null;
var compile_type = null;

var compile_features = [];

for(var i=2; i<args.length - 1; i++) {
    switch(args[i]) {
        case '--lang':
            i++;
            if(i<args.length) compile_lang = args[i];
            break;
        case '--type':
            i++;
            if(i<args.length) compile_type = args[i];
            break;
        case '--feature':
            i++;
            if(i<args.length) compile_features.push(args[i]);
            break;
        case '--name':
            i++;
            if(i<args.length) document_name = args[i];
            break;
        default:
            compile_files.push(args[i]);
            break;
    }
}

var document_path = args[args.length - 1];
if(null === document_name) document_name = path.parse(document_path).name;
var document_dir = path.dirname(document_path);

var generator_configs = [];

if (!fs.existsSync(document_dir)){
    fs.mkdirSync(document_dir, { recursive: true });
}

fs.writeFileSync(document_path,
    JSON.stringify(
        merge.all( compile_files.map(file => JSON.parse(fs.readFileSync(file))) )
    )
)

var component = {
    "name": document_name
    //"customComponent": "./src/custom-test-component.js",
    //"customType": "client"
};
if(compile_type !== null) component["type"] = compile_type;
if(compile_lang !== null) component["language"] = compile_lang;
generator_configs.push( {
    "openrpcDocument": document_path,
    "outDir": document_dir,
    "components": [ component ]
});

(async () => {
    for(var i=0; i < generator_configs.length; i++) {
        const config = generator_configs[i];
        console.log(config);
        await generator.default(config);
    }
})()
