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

    static affectedSectorIdsFromCoords(x: number, y: number, r: number = 50): SectorId[] {
        const boundingSectorIds: { [key: string]: boolean } = {
          [Sector.idFromCoords(x, y)]: true,
        };

        // Top Left
        boundingSectorIds[Sector.idFromCoords(x - r, y - r)] = true;

        // Top Right
        boundingSectorIds[Sector.idFromCoords(x + r, y - r)] = true;

        // Bottom Left
        boundingSectorIds[Sector.idFromCoords(x - r, y + r)] = true;

        // Bottom Right
        boundingSectorIds[Sector.idFromCoords(x + r, y + r)] = true;

        return Object.keys(boundingSectorIds).map(stringToSectorId);
    }
}

function stringToSectorId(str: string): SectorId {
    return (str as any) | 0;
}
