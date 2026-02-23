import gdal
#import osgeo.gdal

# specify GeoTIFF file name, open it using GDAL and get the first band
fn = '386_GDAL_BM9.tif'
ds = gdal.Open(fn, 1)
band = ds.GetRasterBand(1)

# create color table
colors = gdal.ColorTable()

# set color for each value
colors.SetColorEntry(1, (112, 153, 89))
colors.SetColorEntry(2, (242, 238, 162))
colors.SetColorEntry(3, (242, 206, 133))
colors.SetColorEntry(4, (194, 140, 124))
colors.SetColorEntry(5, (214, 193, 156))

# set color table and color interpretation
band.SetRasterColorTable(colors)
band.SetRasterColorInterpretation(gdal.GCI_PaletteIndex)

# close and save file
del band, ds


