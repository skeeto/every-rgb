/* Utilities */

Array.prototype.shuffle = function(){
    for(var j, x, i = this.length; i;
        j = Math.floor(Math.random() * i),
        x = this[--i], this[i] = this[j], this[j] = x);
    return this;
};

/* Color Prototype */

/**
 * Represents a single color, suitable for Set.
 */
function Color(r, g, b) {
    this.r = r;
    this.g = g;
    this.b = b;
}

/**
 * @param {Color} c
 * @returns {Number} the distance from c
 */
Color.prototype.dist = function(c) {
    /* Naive implementation! */
    return Math.abs(this.r - c.r) +
        Math.abs(this.g - c.g) +
        Math.abs(this.b - c.b);
};

/**
 * @returns {string} a string representation suitable for Set
 */
Color.prototype.toString = function() {
    return this.r + ',' + this.g + ',' + this.b;
};

/* Set Prototype */

/**
 * A set data structure that keys off of toString().
 * @param {Array} [elements]
 */
function Set(elements) {
    this.elements = {};
    this.length = 0;
    if (elements != null) {
        this.addAll(elements);
    }
}

/**
 * @param element to add/replace
 * @returns element
 */
Set.prototype.add = function(element) {
    if (!this.has(element)) {
        this.length++;
    }
    this.elements[element] = element;
    return element;
};

/**
 * @param {Array} elements to be added
 * @returns this
 */
Set.prototype.addAll = function(elements) {
    for (var i = 0; i < elements.length; i++) {
        this.add(elements[i]);
    }
    return this;
};

/**
 * @param element to be removed
 * @returns element
 */
Set.prototype.remove = function(element) {
    delete this.elements[element];
    this.length--;
    return element;
};

/**
 * @returns {boolean} true if element is in this set
 */
Set.prototype.has = function(element) {
    return element in this.elements;
};

/**
 * @param {Function} f called for each element
 */
Set.prototype.each = function(f) {
    for (var element in this.elements) {
        f(this.elements[element]);
    }
};

/**
 * @returns {Array} this set as an array
 */
Set.prototype.asArray = function() {
    var array = [];
    this.each(function(element) {
        array.push(element);
    });
    return array;
};

/**
 * @returns a random element
 */
Set.prototype.random = function() {
    var keys = Object.keys(this.elements);
    return this.elements[keys[Math.floor(Math.random() * keys.length)]];
};

/**
 * @returns a random element, removing it from the set
 */
Set.prototype.popRandom = function() {
    var e = this.random();
    this.remove(e);
    return e;
};

/* Pixel Prototype */

/**
 * Represents a pixel in the final result, suitable for Set.
 */
function Pixel(x, y, color) {
    this.x = Math.floor(x);
    this.y = Math.floor(y);
    this.color = color;
    this._neighbors = {};
}

/**
 * @returns {string} a string suitable for Set
 */
Pixel.prototype.toString = function() {
    return '(' + this.x + ',' + this.y + ')';
};

/**
 * @private
 */
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

/**
 * @param {Number} mx x-maximum
 * @param {Number} my y-maximum
 * @returns {Array} an array of this pixel's neighbors within bounds
 */
Pixel.prototype.neighbors = function(mx, my) {
    var key = [mx, my].toString();
    if (this._neighbors[key] != null) {
        return this._neighbors[key];
    } else {
        return this._neighbors[key] = this._findneighbors(mx, my);
    }
};

/* Color Lists */

/**
 * @param {Number} channelbits number of bits per channel
 * @returns {Set} all possible colors (2^(channelbits*3) total)
 */
function colors(channelbits) {
    var set = [];
    var s = Math.pow(2, 8 - channelbits);
    for (var r = 0; r < 256; r += s) {
        for (var g = 0; g < 256; g += s) {
            for (var b = 0; b < 256; b += s) {
                set.push(new Color(r, g, b));
            }
        }
    }
    return set;
}

/* Painter Prototype */

/**
 * Renders a fancy coloration across different interpeter turns.
 * @param {Number} w
 * @param {Number} h
 * @param {Number} channelbits
 */
function Painter(w, h, channelbits) {
    this.w = w;
    this.h = h;
    this.pixels = new Set();
    this.edges = new Set();
    this.colors = colors(channelbits).shuffle();
    this.edges.add(new Pixel(w / 2, h / 2, this.colors.pop()));
    this.count = 1;
}

/**
 * @returns {boolean} true if this Painter is complete
 */
Painter.prototype.isDone = function() {
    return this.count >= this.w * this.h;
};

/**
 * @param {Number} n number of pixels to render before returning
 */
Painter.prototype.render = function(n) {
    n = n || 1;
    console.log('Completed ' + this.count);
    for (var i = 0; i < n && !this.isDone(); i++) {
        this.count++;
        var next = this.colors.pop(),
            places = this.edges.asArray();
        places.forEach(function(p) {
            p._dist = p.color.dist(next);
        });
        places.sort(function(a, b) {
            return b._dist - a._dist;
        });
        var found = false;
        while (!found) {
            var best = places.pop();
            var neighbors = best.neighbors(this.w, this.h).shuffle();
            for (var j = 0; j < neighbors.length; j++) {
                var p = neighbors[j];
                if (!this.edges.has(p) && !this.pixels.has(p)) {
                    p.color = next;
                    this.edges.add(p);
                    found = true;
                    break;
                }
            }
            if (!found) {
                this.edges.remove(best);
                this.pixels.add(best);
            }
        }
    }
    if (this.isDone()) {
        var _this = this;
        this.edges.each(function(p) {
            _this.pixels.add(p);
        });
    }
};

/**
 * Draw the current results.
 * @param {CanvasRenderingContext2D} ctx
 */
Painter.prototype.draw = function(ctx) {
    var w = this.w, h = this.h, data = ctx.getImageData(0, 0, w, h);
    this.pixels.each(function(p) {
        var c = p.color;
        data.data[4 * w * p.y + 4 * p.x + 0] = c.r;
        data.data[4 * w * p.y + 4 * p.x + 1] = c.g;
        data.data[4 * w * p.y + 4 * p.x + 2] = c.b;
    });
    ctx.putImageData(data, 0, 0);
};

/**
 * Completely render this Painter across many turns.
 * @param {CanvasRenderingContext2D} ctx
 * @param {Numnber} [step=64] number of pixels to render per step
 */
Painter.prototype.run = function(ctx, step) {
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, this.w, this.h);
    step = step || 64;
    var _this = this;
    var timer = window.setInterval(function() {
        try {
            _this.render(step);
            _this.draw(ctx);
        } catch (e) {
            window.clearInterval(timer);
            throw e;
        }
        if (_this.done) {
            window.clearInterval(timer);
        }
    }, 1);
};

window.addEventListener('load', function() {
    var n = 512;
    var canvas = document.querySelector('#canvas');
    var ctx = canvas.getContext('2d');
    new Painter(n, n, 6).run(ctx, 16);
});
