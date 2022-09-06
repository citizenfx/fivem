import { Injectable } from "@angular/core";

export class ModInfo {
    modId: number;
    displayName: string;
    authorName: string;
    iconUri: string;
    sizeBytes: number;
    enabled: boolean;
    downloadState: 'downloading' | 'available' | 'installed';
    progress: number;
}

@Injectable()
export class ModsService {
}
