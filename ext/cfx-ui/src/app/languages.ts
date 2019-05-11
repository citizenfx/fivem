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
