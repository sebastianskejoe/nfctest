package main

import (
    "encoding/xml"
    "os"
    "log"
    "text/template"
//    "strconv"
    "fmt"
)

type Arg struct {
    Name string
    Type string
    Typecode string
    Default string
    Length string
}

type Response struct {
    Args []Arg
}

type Command struct {
    Name string
    Code string
    Args []Arg
    Response Response
}

type XMLArg struct {
    Commands []Command
}

func main() {
    protocol, err := os.Open("PN532.xml")
    if err != nil {
        log.Fatal(err)
    }

    // Parse protocol
    commands := make([]Command, 0)
    var cmd Command
    var resp Response
    inResp := false

    // Loop through each token
    decoder := xml.NewDecoder(protocol)
    for token, err := decoder.Token(); err == nil; token, err = decoder.Token() {
        switch elem := token.(type) {

        // Check for start element
        case xml.StartElement:
            switch elem.Name.Local {
            case "command": // Start of new command
                cmd = newCommand(elem.Attr)
            case "arg":
                arg := newArg(elem.Attr)

                // Check if the arg belongs to the response
                if inResp {
                    println("added arg to resp ", cmd.Name)
                    resp.Args = append(resp.Args, arg)
                } else {
                    cmd.Args = append(cmd.Args, arg)
                }
            case "response":
                resp = newResponse(cmd.Name, elem.Attr)
                inResp = true
            }

        // Check for end element
        case xml.EndElement:
            switch elem.Name.Local {
            case "command":
                commands = append(commands, cmd)
            case "response":
                cmd.Response = resp
                inResp = false
            }
        }
    }

    // Print commands
    commandsfile, err := os.Create("NFC.cpp")
    if err != nil {
        log.Fatal(err)
    }
    headerfile, err := os.Create("NFC.h")
    if err != nil {
        log.Fatal(err)
    }

    tmpl := template.Must(template.New("cmd").Parse(cmdTemplate))
    tmpl.Execute(commandsfile, XMLArg{commands})

    tmpl = template.Must(template.New("header").Parse(headerTemplate))
    tmpl.Execute(headerfile, XMLArg{commands})
}

func newCommand(attr []xml.Attr) Command {
    var cmd Command
    for _, value := range attr {
        switch value.Name.Local {
        case "name":
            cmd.Name = value.Value
            fmt.Println("Command: ",cmd.Name)
        case "code":
            cmd.Code = value.Value
        }
    }
    return cmd
}

func newResponse(name string, attr []xml.Attr) Response {
/*    var cmd Command
    cmd.Name = fmt.Sprintf("%sResponse", name)
    for _, value := range attr {
        switch value.Name.Local {
        case "name":
            cmd.Name = value.Value
        case "code":
            cmd.Code = value.Value
        }
    }
    return cmd*/
    var resp Response
    return resp
}

func newArg(attr []xml.Attr) Arg {
    var arg Arg
    for _, value := range attr {
        switch value.Name.Local {
        case "name":
            arg.Name = value.Value
        case "type":
            switch value.Value {
            case "u":
                arg.Typecode = value.Value
                arg.Type = "unsigned char"
            case "a":
                arg.Typecode = value.Value
                arg.Type = "unsigned char *"
            }
        case "default":
            arg.Default = value.Value
        case "length":
            arg.Length = value.Value
        }
    }
    return arg
}

var cmdTemplate string = `#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>
#include "NFC.h"

#define HDR_LENGTH 6

void setDebugLCD(LiquidCrystal *lcd) {
    _dbgLCD = lcd;
}

void requestHeader(unsigned char *request, unsigned char length) {
    request[0] = 0x00;
    request[1] = 0x00;
    request[2] = 0xFF;
    request[3] = length+1;
    request[4] = 0xFF-length;
    request[5] = 0xD4;
}

unsigned char requestDCS(unsigned char *data, unsigned char length) {
    unsigned char ret = 0xFF;
    int i;
    ret -= 0xD4;
    for (i = 0; i < length; i++) {
        ret -= data[i];
    }
    ret += 1;
    return ret;
}

void sendRequest(char *name, unsigned char code, char *signature, ...) {
    int i;
    unsigned char length = 0, u;
    unsigned char *array;
    va_list args;

    unsigned char requestdata[255];
    unsigned char data[255];
    memset(requestdata, 0, 255);
    memset(data, 0, 255);

    va_start(args, signature);
    data[length] = code;
    length += sizeof(code);
    for (i = 0; i < strlen(signature); i++) {
        switch(signature[i]) {
        case 'u':
            u = (unsigned char)va_arg(args, int);
            data[length] = u;
            length += sizeof(u);
            break;
        case 'a':
            array = va_arg(args, unsigned char*);
            if(array != 0) {
                memcpy(&data[length], array, strlen((const char*)array));
                length += strlen((const char *)array);
            }
            break;
        }
    }
    va_end(args);

    // Create request header in requestData
    requestHeader(requestdata, length);

    // Copy data to requestData and add DCS
    memcpy(&requestdata[HDR_LENGTH], &data[0], length);
    requestdata[HDR_LENGTH+length] = requestDCS(data, length);

    // Write data to serial
    for (i = 0; i < length+HDR_LENGTH+1; i++) {
        if (_dbgLCD) {
            _dbgLCD->print(requestdata[i], HEX);
        }
        Serial.write(requestdata[i]);
    }
    Serial.flush();
}

// BEGIN generated code
{{range .Commands}}{{if len .Response.Args}}{{.Name}}Response * {{else}}void {{end}}{{.Name}}({{range $i, $e := .Args}}{{if $i}}, {{end}}{{.Type}} {{.Name}}{{end}}) {
    sendRequest("{{.Name}}", {{.Code}}, "{{range .Args}}{{.Typecode}}{{end}}"{{range $i, $e := .Args}}, {{.Name}}{{end}});{{ if len .Response.Args }}

    return ({{.Name}}Response *)readResponse("{{range .Response.Args}}{{.Typecode}}{{if .Length}}{{.Length}}{{end}}{{end}}");{{end}}
}

{{end}}// END generated code
`

var headerTemplate string = `#ifndef __NFC_H
#define __NFC_H

#include <LiquidCrystal.h>

static LiquidCrystal *_dbgLCD = 0;

struct Response {
    unsigned char code;
    void *data;
};

void setDebugLCD(LiquidCrystal *lcd);

void requestHeader(unsigned char *request, unsigned char length);
unsigned char requestDCS(unsigned char *data, unsigned char length);
void sendRequest(char *name, unsigned char code, char *signature, ...);

void * readResponse(char * signature);

// BEGIN generated response structs
{{range .Commands}}{{if len .Response.Args}}
typedef struct { {{range .Response.Args}}
    {{.Type}} {{.Name}};{{end}}
} {{.Name}}Response;
{{end}}{{end}}
// END generated response structs


// BEGIN generated requests{{range .Commands}}
{{if len .Response.Args}}{{.Name}}Response * {{else}}void {{end}}{{.Name}}({{range $i, $e := .Args}}{{if $i}}, {{end}}{{.Type}} {{.Name}}{{if .Default}} = {{.Default}}{{end}}{{end}});{{end}}
// END generated requests
#endif`
