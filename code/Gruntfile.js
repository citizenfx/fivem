module.exports = function(grunt)
{
	var fs = require('fs');

	grunt.initConfig({
		prompt: {
			'component-new': {
				options: {
					questions: [
						{
							config:		'component.name',
							type: 		'input',
							message: 	'Component name',
							validate:   function(v) { return v.match(/^[a-z0-9\-:]+$/) ? true : 'Component names must only contain alphanumerics or dashes.'; }
						}
					]
				}
			}
		}
	});

	grunt.loadNpmTasks('grunt-prompt');

	grunt.registerTask('component-new', 'Creates a new component.', function()
	{
		var name = grunt.config.get('component.name');
		var rawName = name;
		name = name.replace(/:/g, '-');

		if (!fs.existsSync('components/'))
		{
			fs.mkdirSync('components/');
		}

		fs.mkdirSync('components/' + name);

		fs.appendFileSync('components/config.lua', grunt.template.process("component '<%= name %>'\r\n", { data: { name: name } }))

		fs.writeFileSync('components/' + name + '/component.lua', '');
		fs.writeFileSync('components/' + name + '/component.json', JSON.stringify({
			'name': rawName,
			'version': '0.1.0',
			'dependencies': [
				'fx[2]'
			],
			'provides': []
		}, null, '\t'));

		fs.writeFileSync('components/' + name + '/component.rc', "fxComponent 115 component.json\r\n")

		fs.mkdirSync('components/' + name + '/include');
		fs.mkdirSync('components/' + name + '/src');
		fs.mkdirSync('components/' + name + '/tests');

		fs.writeFileSync('components/' + name + '/include/Local.h', "#pragma once\r\n");

		fs.writeFileSync('components/' + name + '/src/Component.cpp', fs.readFileSync('components/template/Component.cpp'));

		grunt.log.ok('Component created.');
	});

	grunt.registerTask('component:new', ['prompt:component-new', 'component-new']);
};
