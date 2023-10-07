import AlwCore from './lib/AlwEntry.js';
import AlwCoreModule from './lib/AlwEntry.wasm';

export async function alwGetRuntime() {
    let modx = await new Promise((resolve) => {
        const module = AlwCore({
            locateFile(path) {
              if(path.endsWith('.wasm')) {
                return AlwCoreModule;
              }
              return path;
            },
            onRuntimeInitialized () {
                console.log(module)
                window.xxxx = module
                resolve(module);
            }
        });
    })
    window.alw = await modx;
    return modx;
}
