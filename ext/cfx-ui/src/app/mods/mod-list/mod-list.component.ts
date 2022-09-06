import { Component, OnInit } from '@angular/core';
import { ModInfo } from '../mods.service';

@Component({
	selector: 'app-mod-list',
	templateUrl: './mod-list.component.html',
	styleUrls: ['./mod-list.component.scss']
})
export class ModListComponent implements OnInit {
	installedMods: ModInfo[] = [];
	availableMods: ModInfo[] = [];

	constructor() { }

	ngOnInit(): void {
		this.installedMods = [
			{
				displayName: 'World of Worlds',
				sizeBytes: 368 * 1024 * 1024,
				authorName: 'Smello',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1159087,
				downloadState: 'installed',
				enabled: true,
				progress: 1
			},
			{
				displayName: 'World of Scripts',
				sizeBytes: 469 * 1024 * 1024,
				authorName: 'Smello',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1159084,
				downloadState: 'downloading',
				enabled: true,
				progress: 0.42
			},
			{
				displayName: 'R* collision fix',
				sizeBytes: 69 * 1024,
				authorName: 'Smallo (again)',
				iconUri:
					`https://upload.wikimedia.org/wikipedia/commons/thumb/0/03/Rockstar_North_Logo.svg/512px-Rockstar_North_Logo.svg.png`,
				modId: 1159066,
				downloadState: 'installed',
				enabled: false,
				progress: 0.42
			},
		];

		this.availableMods = [
			{
				displayName: 'World of Globals',
				sizeBytes: 42 * 1024 * 1024,
				authorName: 'Smello',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1159088,
				downloadState: 'available',
				enabled: false,
				progress: 1
			},
			{
				displayName: 'World of Worlds',
				sizeBytes: (1337 * 1024 * 1024) + (1500 * 1024),
				authorName: 'Smollo',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1159089,
				downloadState: 'available',
				enabled: false,
				progress: 1
			},
			{
				displayName: 'World of Worlds',
				sizeBytes: (1337 * 1024 * 1024) + (1500 * 1024),
				authorName: 'Smollo',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1139089,
				downloadState: 'available',
				enabled: false,
				progress: 1
			},
			{
				displayName: 'World of Worlds',
				sizeBytes: (1337 * 1024 * 1024) + (1500 * 1024),
				authorName: 'Smollo',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1059089,
				downloadState: 'available',
				enabled: false,
				progress: 1
			},
			{
				displayName: 'World of Worlds',
				sizeBytes: (1337 * 1024 * 1024) + (1500 * 1024),
				authorName: 'Smollo',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1159589,
				downloadState: 'available',
				enabled: false,
				progress: 1
			}, {
				displayName: 'World of Worlds',
				sizeBytes: (1337 * 1024 * 1024) + (1500 * 1024),
				authorName: 'Smollo',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1959589,
				downloadState: 'available',
				enabled: false,
				progress: 1
			}, {
				displayName: 'World of Worlds',
				sizeBytes: (1337 * 1024 * 1024) + (1500 * 1024),
				authorName: 'Smollo',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1151589,
				downloadState: 'available',
				enabled: false,
				progress: 1
			}, {
				displayName: 'World of Worlds',
				sizeBytes: (1337 * 1024 * 1024) + (1500 * 1024),
				authorName: 'Smollo',
				iconUri:
					`https://vignette.wikia.nocookie.net/gtawiki/images/d/d0/Rank-100.png/revision/latest/scale-to-width-down/340?cb=20140704160357`,
				modId: 1189589,
				downloadState: 'available',
				enabled: false,
				progress: 1
			},
		];
	}

}
