export const SectorSize = 282;

type SpatialId = number;

export type SectorId = number;

export class Sector {
    private static spatialIdFromCoord(coord: number): number {
        const fc = coord < 0 ? coord - SectorSize : coord;
        return 500 + ((fc / SectorSize) | 0);
    }

    private static coordFromSectorSpatialId(spatialId: SpatialId): number {
        return (spatialId - 500) * SectorSize;
    }

    private static idFromSpatialIds(sx: number, sy: number): number {
        return sx * 1000 + sy;
    }

    private static spatialIdsFromId(id: SectorId): [SpatialId, SpatialId] {
        const sx = Math.floor(id / 1000);
        const sy = id % 1000;

        return [sx, sy];
    }

    static idFromCoords(x: number, y: number): SectorId {
        return Sector.spatialIdFromCoord(x) * 1000 + Sector.spatialIdFromCoord(y);
    }

    static affectedSectorIdsFromCoords(x: number, y: number, r: number = SectorSize/2): SectorId[] {
        const sectors: { [key: string]: boolean } = {};

        // top left                   top center                  top right
        addSector(sectors, x-r, y-r); addSector(sectors, x, y-r); addSector(sectors, x+r, y-r);

        // center left                center center               center right
        addSector(sectors, x-r, y);   addSector(sectors, x, y);   addSector(sectors, x+r, y);

        // bottom left                bottom center               bottom right
        addSector(sectors, x-r, y+r); addSector(sectors, x, y+r); addSector(sectors, x+r, y+r);

        return Object.keys(sectors).map(stringToSectorId);
    }
}

function addSector(sectors: { [key: string]: boolean }, x: number, y: number) {
  sectors[Sector.idFromCoords(x, y)] = true;
}

function stringToSectorId(str: string): SectorId {
    return (str as any) | 0;
}
