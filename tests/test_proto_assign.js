// http://forum.espruino.com/conversations/371845/#comment16354695
var Collection = function Collection(value) {
  return value;
};
var KeyedCollection = /*@__PURE__*/ (function (Collection) {
  function KeyedCollection(value) {
    //return isKeyed(value) ? value : KeyedSeq(value);
  }
  if (Collection) KeyedCollection.__proto__ = Collection;
  KeyedCollection.prototype = Object.create(Collection && Collection.prototype);
  KeyedCollection.prototype.constructor = KeyedCollection;
  return KeyedCollection;
})(Collection);
var IndexedCollection = /*@__PURE__*/ (function (Collection) {
  function IndexedCollection(value) {
    //return isIndexed(value) ? value : IndexedSeq(value);
  }
  if (Collection) IndexedCollection.__proto__ = Collection;
  IndexedCollection.prototype = Object.create(Collection && Collection.prototype);
  IndexedCollection.prototype.constructor = IndexedCollection;
  return IndexedCollection;
})(Collection);
// it would have failed previously
result=1;
