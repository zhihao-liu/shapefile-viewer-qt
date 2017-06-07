#ifndef NSDEF_H
#define NSDEF_H

#include <list>
#include <memory>

namespace cl
{
namespace Dataset
{
class ShapeDatasetShared;
class ShapeRecordUnique;
}

namespace Graphics
{
class Shape;

class Point;
class MultiPartShape;
class Polyline;
class Polygon;

class GraphicAssistant;
}

namespace DataManagement
{
class ShapeDoc;

class ShapeFactory;
enum class ShapeProvider;
class ShapeFactoryEsri;

class DisplayManager;
class Observer;

class ShapeView;
class ShapeViewObserver;
}

namespace Map
{
class Map;
enum class MapStyle;
class MapBuilder;
class MapDirector;
class MapObserver;

class MapElement;
class GridLine;
class NorthPointer;
class ScaleBar;
}

template<typename T> class Pair;
template<typename T> class Rect;

typedef std::list<std::shared_ptr<Graphics::Shape>>::iterator LayerIterator;
}
#endif // NSDEF_H
