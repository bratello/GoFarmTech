#!/usr/bin/env node
var fs = require('fs');
const util = require('util');
const readdir = util.promisify(fs.readdir);
const readFile = util.promisify(fs.readFile);
const exec = util.promisify(require('child_process').exec);

async function make() {
    console.log("Current dir: " + __dirname);
    var content = await readFile(__dirname + "/index.html", "utf8");
    function replaceScriptContent(bpos, epos, fileName, fileContent) {
        content = content.substring(0, bpos) 
                + "\r\n<script>\r\n/*replace " + fileName + "*/\r\n" 
                + fileContent
                + "\r\n</script>\r\n" 
                + content.substring(epos);
    }

    function replaceCssContent(bpos, epos, fileName, fileContent) {
        content = content.substring(0, bpos) 
                + "\r\n<style>\r\n/*replace " + fileName + "*/\r\n" 
                + fileContent
                + "\r\n</style>\r\n" 
                + content.substring(epos);
    }

    function replaceFontContent(bpos, epos, fileName, fileContent) {
        var ret = content.substring(0, bpos) 
                + "/*" + fileName + "*/"
                + "url(data:application/font-woff2;charset=utf-8;base64,"; 
        ret += fileContent + ")" + content.substring(epos);
        content = ret;
    }

    async function fontReader(filePath) {
        var ret = await readFile(filePath);
        return ret.toString('base64');
    }

    async function handleFiles(files, begin, end, root, replacer, reader) {
        if(!replacer) {
            replacer = replaceScriptContent;
        }
        if(!reader) {
            reader = async function (filePath) {
                return await readFile(filePath, "utf-8");
            };
        }
        for(var j = 0; j < files.length; j++) {
            var fileName = files[j];
            fileName = root + fileName.trim();
            var pos = content.indexOf(fileName);
            if(pos == -1) {
                console.error("Unknown file: " + fileName);
                continue;
            }
            console.log("Handle file: " + fileName);
            var bpos = content.lastIndexOf(begin, pos);
            var epos = content.indexOf(end, pos) + end.length;
            replacer(bpos, epos, files[j], await reader(__dirname + "/" + fileName));
        }
    };
    var folders  = {
        src: {
            files: [],
            handle: async function() {
                console.log("Handle scripts...");
                await handleFiles(this.files, "<script", "</script>", "src/");
            }
        },
        css: {
            files: [],
            handle: async function() {
                console.log("Handle css...");
                await handleFiles(this.files, "<link", ">", "css/", replaceCssContent);
            }
        },
        font: {
            files: [],
            handle: async function() {
                console.log("Handle fonts...");
                await handleFiles(this.files, "url(", ")", "font/", replaceFontContent, fontReader);
            }
        }
    };

    var keys = Object.keys(folders);
    for(var i = 0; i < keys.length; i++) {
        var key = keys[i];
        folders[key].files = await readdir(__dirname + "/" + key, "utf-8");
        await folders[key].handle();
    }

    var buildHtml = __dirname + "/build/index.html";
    if(fs.existsSync(buildHtml)) {
        fs.unlinkSync(buildHtml);
    }
    if(fs.existsSync(buildHtml+'.gz')) {
        fs.unlinkSync(buildHtml+'.gz');
    }
    fs.writeFileSync(buildHtml, content, "utf-8");

    await exec('gzip ' + buildHtml);
}
make();