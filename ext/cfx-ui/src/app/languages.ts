import { L10nSchema } from "angular-l10n";

const languages = [
    {
        name: 'en',
        displayName: 'English'
    },
    {
        name: 'fr',
        displayName: 'Français'
    },
    {
        name: 'nl',
        displayName: 'Nederlands'
    },
    {
        name: 'da',
        displayName: 'Dansk'
    },
    {
        name: 'de',
        displayName: 'Deutsch'
    },
    {
        name: 'pl',
        displayName: 'Polski'
    },
    {
        name: 'it',
        displayName: 'Italiano'
    },
    {
        name: 'pt',
        displayName: 'Português'
    },
    {
        name: 'ru',
        displayName: 'Русский'
    },
    {
        name: 'es',
        displayName: 'Español'
    },
    {
        name: 'cs',
        displayName: 'Čeština'
    },
    {
        name: 'zh',
        displayName: '繁體中文'
    },
	{
        name: 'cn',
        displayName: '简体中文'
    },
];

export class Languages {
    static toList(): L10nSchema[] {
        return languages.map(({name, displayName}) => ({
            locale: {
                language: name
            },
            dir: 'ltr',
            text: displayName
        }));
    }

    static toSettingsOptions() {
        const options: {[code: string]: string} = {};

        const languagesSet = languages;
        languagesSet.sort((a, b) => {
            return a.displayName.localeCompare(b.displayName);
        });

        for (const lang of languagesSet) {
            options[lang.name] = lang.displayName;
        }

        return options;
    }
}
