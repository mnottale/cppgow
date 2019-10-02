#! /usr/bin/env python3

import re
import sys


def generate_route(route, rtype, fname, fargs):
    dargs = dict()
    for a in fargs:
        dargs[a[1]] = a[0]
    sc = route.find(':')
    if sc == -1:
        sc = len(route)
    route_prefix = route[0:sc]
    route_regex = route_prefix
    route_args = list()
    while sc < len(route):
        name = re.search('[a-zA-Z0-9]+', route[sc+1:]).group()
        nextp = sc + 1 + len(name)
        route_args.append(name)
        typ = dargs[name]
        if typ == 'std::string':
            if nextp == len(route):
              route_regex += '(.*)'
            else:
              route_regex += '([^' + route[nextp] + ']*)'
        elif typ in ['double', 'float']:
            route_regex += '([-+]?[0-9]*\.[0-9]*)'
        else:
            route_regex += '([0-9]+)'
        p = route[nextp:].find(':')
        if p == -1:
            route_regex += route[nextp:]
            break
        p += nextp
        route_regex += route[nextp:p]
        sc = p
    #print("REX " + route_regex)
    code = 'void serve_' + fname + '(cppgow::ServerRequest req, cppgow::ServerResponseWriter& responseWriter) {\n'
    for a in range(len(route_args)):
        code += '  req.query["' + route_args[a] + '"] = req.parameters[' + str(a) + '];\n'
    code += '  router::response(responseWriter, ' + fname + '('
    for a in fargs:
        code += 'router::cast<' + a[0] + '>(req.query["' + a[1] + '"]),'
    code = code[:-1] + '));\n'
    code += '}\n'
    regis = '  router::registerRoute("' + route_prefix + '", "' + route_regex + '", serve_' + fname + ');\n'
    return (code, regis)

def process_file(fname):
    gcode = ''
    gregis = ''
    with open(fname, 'r') as f:
        lines = f.readlines()
        for li in range(len(lines)):
            l = lines[li]
            m = re.search('@route\(([^)]*)\)', l)
            if m is not None:
                route = m.group(1)
                decl = lines[li+1]
                #print(decl)
                m = re.search('^([^ ]+)\s+([^ (]+)\s*\(([^)]*)\)', decl)
                (rtype, name, rawargs) = m.group(1, 2, 3)
                args = list()
                for sa in rawargs.split(','):
                    args.append(sa.strip().split(' '))
                #m = re.search('(([^ ]+)\s+([a-zA-Z0-9_]+)(,|$))*', rawargs)
                #print(m.groups())
                (code, regis) = generate_route(route, rtype, name, args)
                gcode += code
                gregis += regis
    return (gcode, gregis)

(code, regis) = process_file(sys.argv[1])
print('#include "cppgow/router.hh"\n')
print(code)
print('__attribute__((constructor)) static void router_register_routes() {\n')
print(regis)
print('}\n')
               