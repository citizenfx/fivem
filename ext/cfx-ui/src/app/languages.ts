import { Language } from 'angular-l10n';

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
        name: 'pl',
        displayName: 'Polski'
    },
];

export class Languages {
    static toList() {
        return languages.map(({name}) => ({
            code: name,
            dir: 'ltr'
        }));
    }

    static toSettingsOptions() {
        const options: {[code: string]: string} = {};

        for (const lang of languages) {
            options[lang.name] = lang.displayName;
        }

        return options;
    }
}
