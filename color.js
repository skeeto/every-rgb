/* Utilities */

var $ = function(s) { return document.querySelector(s); };
var ctx = $('canvas').getContext('2d');
Array.prototype.shuffle = function(){
    for(var j, x, i = this.length; i;
        j = Math.floor(Math.random() * i),
        x = this[--i], this[i] = this[j], this[j] = x);
    return this;
};

function clear() {
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, 128, 128);
}

/* Color Prototype */

function Color(r, g, b) {
    this.r = r;
    this.g = g;
    this.b = b;
}

Color.prototype.dist = function(c) {
    return Math.abs(this.r - c.r) +
        Math.abs(this.g - c.g) +
        Math.abs(this.b - c.b);
};

Color.prototype.toString = function() {
    return this.r + ',' + this.g + ',' + this.b;
};

/* Set Prototype */

function Set(elements) {
    this.elements = {};
    this.length = 0;
    if (elements != null) {
        this.addAll(elements);
    }
}

Set.prototype.add = function(element) {
    if (!this.has(element)) {
        this.length++;
    }
    this.elements[element] = element;
    return element;
};

Set.prototype.addAll = function(elements) {
    for (var i = 0; i < elements.length; i++) {
        this.add(elements[i]);
    }
    return this;
};

Set.prototype.remove = function(element) {
    delete this.elements[element];
    this.length--;
    return element;
};

Set.prototype.has = function(element) {
    return element in this.elements;
};

Set.prototype.each = function(f) {
    for (var element in this.elements) {
        f(this.elements[element]);
    }
};

Set.prototype.asArray = function() {
    var array = [];
    this.each(function(element) {
        array.push(element);
    });
    return array;
};

Set.prototype.random = function() {
    var keys = Object.keys(this.elements);
    return this.elements[keys[Math.floor(Math.random() * keys.length)]];
};

Set.prototype.popRandom = function() {
    var e = this.random();
    this.remove(e);
    return e;
};

/* Pixel Prototype */

function Pixel(x, y, color) {
    this.x = Math.floor(x);
    this.y = Math.floor(y);
    this.color = color;
    this._neighbors = {};
}

Pixel.prototype.toString = function() {
    return '(' + this.x + ',' + this.y + ')';
};

Pixel.prototype._findneighbors = function(mx, my) {
    var pixels = [];
    for (var x = -1; x <= 1; x++) {
        for (var y = -1; y <= 1; y++) {
            if (x !== 0 || y !== 0) {
                var xx = this.x + x, yy = this.y + y;
                if (xx >= 0 && xx < mx && yy >= 0 && yy < my) {
                    pixels.push(new Pixel(xx, yy));
                }
            }
        }
    }
    return pixels;
};

Pixel.prototype.neighbors = function(mx, my) {
    var key = [mx, my].toString();
    if (this._neighbors[key] != null) {
        return this._neighbors[key];
    } else {
        return this._neighbors[key] = this._findneighbors(mx, my);
    }
};

/* Color Lists */

var colors = (function() {
    var colors = [], s = 8;
    for (var r = 0; r < 256; r += s) {
        for (var g = 0; g < 256; g += s) {
            for (var b = 0; b < 256; b += s) {
                colors.push(new Color(r, g, b));
            }
        }
    }
    return colors;
}());

/* Painter Prototype */

function Painter(w, h) {
    this.w = w;
    this.h = h;
    this.pixels = new Set();
    this.edges = new Set();
}

Painter.prototype.render = function(w, h, colors) {
    var pixels = new Set(),
        edges = new Set(),
        colorset = new Set(colors);
    edges.add(new Pixel(w / 2, h / 2, colorset.popRandom()));
    for (var i = 0; i < w * h - 1; i++) {
        var next = colorset.popRandom();
        var places = edges.asArray();
        places.forEach(function(p) {
            p._dist = p.color.dist(next);
        });
        places.sort(function(a, b) {
            return b._dist - a._dist;
        });
        var found = false;
        while (!found) {
            var best = places.pop();
            var neighbors = best.neighbors(w, h).shuffle();
            for (var j = 0; j < neighbors.length; j++) {
                var p = neighbors[j];
                if (!edges.has(p) && !pixels.has(p)) {
                    p.color = next;
                    edges.add(p);
                    found = true;
                    break;
                }
            }
            if (!found) {
                edges.remove(best);
                pixels.add(best);
            }
        }
    }
    edges.each(function(p) {
        pixels.add(p);
    });
    return pixels;
};

function draw(pixels, w, h) {
    var data = ctx.getImageData(0, 0, w, h);
    pixels.each(function(p) {
        var c = p.color;
        data.data[4 * w * p.y + 4 * p.x + 0] = c.r;
        data.data[4 * w * p.y + 4 * p.x + 1] = c.g;
        data.data[4 * w * p.y + 4 * p.x + 2] = c.b;
    });
    ctx.putImageData(data, 0, 0);
};

(function(n) {
    clear();
    draw(render(n, n, colors), n, n);
}(128));
