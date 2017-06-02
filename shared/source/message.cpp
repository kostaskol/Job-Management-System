#include <helpfunc.h>
#include "message.h"
#include "constants.h"

using namespace std;
namespace msg {
    message::message(my_string mes, int mode)
            : _message(mes), _list(), _using_list(false), _mode(mode) {}

    message::message(my_vector<my_string> vec, int mode)
            : _message(""), _list(vec), _using_list(true), _mode(mode) {}

    message::~message() {}

    bool message::encode() {
        _message = "";

        if (!_using_list) {
            cout << "Message: Cannot encode string. Use with list constructor" << endl;
        }
        switch(_mode) {
            // Request protocol
            case PROT_REQ: {
                if (_using_list) {
                    switch (hf::get_com(_list, true)) {
                        case C_SUB: {
                            _message = "0";
                            for (size_t i = 1; i < _list.size(); i++) {
                                _message += _list.at(i);
                                _message += " ";
                            }
                            break;
                        }
                        case C_STAT: {
                            _message = "1";

                            my_string tmp = _list.at(1);
                            char *l_id = new char[6];
                            for (size_t i = 0; i < 5; i++) {
                                l_id[i] = tmp.c_str()[i];
                                if (tmp.c_str()[i] == '\0') break;
                            }
                            _message += l_id;
                            delete[] l_id;
                            break;
                        }
                        case C_STAT_ALL: {
                            _message = "2";

                            if (_list.size() == 1) {
                                _message += '\0';
                                break;
                            }

                            my_string tmp = _list.at(1);
                            char *sec = new char[6];
                            for (size_t i = 0; i < 5; i++) {
                                sec[i] = tmp.c_str()[i];
                                if (tmp.c_str()[i] == '\0') break;
                            }
                            _message += sec;
                            break;
                        }
                        case C_SHOW_ACT: {
                            _message = "3\0";
                            break;
                        }
                        case C_SHOW_POOL: {
                            _message = "4\0";
                            break;
                        }
                        case C_SHOW_FIN: {
                            _message = "5\0";
                            break;
                        }
                        case C_SUSP: {
                            _message = "6";
                            my_string tmp = _list.at(1);
                            char *l_id = new char[6];
                            for (size_t i = 0; i < 5; i++) {
                                l_id[i] = tmp.c_str()[i];
                                if (tmp.c_str()[i] == '\0') break;
                            }
                            _message += l_id;
                            delete[] l_id;
                            break;
                        }
                        case C_RES: {
                            _message = "7";
                            my_string tmp = _list.at(1);
                            char *l_id = new char[6];
                            for (size_t i = 0; i < 5; i++) {
                                l_id[i] = tmp.c_str()[i];
                                if (tmp.c_str()[i] == '\0') break;
                            }
                            _message += l_id;
                            delete[] l_id;
                            break;
                        }
                        case C_SD: {
                            _message = "8\0";
                            break;
                        }
                        default: {
                            if (_list.at(0) == "j") {
                              _message = "j";
                              try {
                                _message += _list.at(1);
                                _message += " ";
                                _message += _list.at(2);
                              } catch (exception &e) {
                                cout << "Encode: Malformed command: \"J\"";
                              }
                            }
                        }
                    }
                } else {
                    cout << "Cannot encode string. Use with the list constructor" << endl;
                    return false;
                }
                break;
            }

            // Response protocol
            case PROT_RESP: {
                if (_using_list) {
                switch(hf::get_com(_list, false)) {
                    case C_SUB: {
                        try {
                            _message = "0";
                            for (size_t i = 1; i < _list.size(); i++) {
                                _message += _list.at(i);
                                _message += " ";
                            }
                            break;
                        } catch (exception& e) {
                            cerr << "Encode: PROT_RESP: Malformed list for submit" << endl;
                            return false;
                        }
                        break;
                    }
                    case C_STAT: {
                        _message = "1";
                        try {
                            if (_list.at(1) == "0") {
                                _message += "0";
                                _message += " ";
                                _message += _list.at(2);
                                break;
                            }
                            _message += _list.at(1);
                            break;
                        } catch (exception &e) {
                            cerr << "Encode: PROT_RESP: Malformed list for status" << endl;
                            return false;
                        }
                    }
                    case C_STAT_ALL: {
                        _message = "2";
                        for (size_t i = 1; i < _list.size(); i++) {
                            try {
                                my_string tmp = _list.at(i);
                                if (tmp == "0") {
                                  _message += "0";
                                  _message += "-";
                                  _message += _list.at(++i);
                                  _message += " ";
                                } else {
                                  _message += tmp;
                                  _message += " ";
                                }
                            } catch (exception &e) {
                                cerr << "Encode: PROT_RESP: Malformed list for status all" << endl;
                                return false;
                            }
                        }
                        break;
                    }
                    case C_SHOW_ACT: {
                        _message = "3";
                        for (size_t i = 1; i < _list.size(); i++) {
                            try {
                                _message += _list.at(i);
                                _message += " ";
                            } catch (exception &e) {
                                cerr << "Encode: PROT_RESP: Malformed list for show active" << endl;
                                return false;
                            }
                        }
                        break;
                    }
                    case C_SHOW_POOL: {
                        _message = "4";
                        for (size_t i = 1; i < _list.size(); i++) {
                            try {
                                // Format looks something like this:
                                // pid-num_jobs
                                _message += _list.at(i);
                                _message += " ";
                            } catch (exception &e) {
                                cerr << "Encode: PROT_RESP: Malformed list for show pools" << endl;
                                return false;
                            }
                        }
                        break;
                    }
                    case C_SHOW_FIN: {
                        _message = "5";
                        for (size_t i = 1; i < _list.size(); i++) {
                            try {
                                _message += _list.at(i);
                                _message += " ";
                            } catch (exception &e) {
                                cerr << "Encode: PROT_RESP: Malformed list for show finished" << endl;
                                return false;
                            }
                        }
                        break;
                    }
                    case C_SUSP: {
                        _message = "6";
                        try {
                            _message += _list.at(1);
                        } catch (exception &e) {
                            cerr << "Encode: PROT_RESP: Malformed list for suspend" << endl;
                            return false;
                        }
                        break;
                    }
                    case C_RES: {
                        _message = "7";
                        try {
                            _message += _list.at(1);
                        } catch (exception &e) {
                            cerr << "Encode: PROT_RESP: Malformed list for resume" << endl;
                            return false;
                        }
                        break;
                    }
                    case C_SD: {
                        _message = "8";
                        try {
                            _message += _list.at(1);
                            _message += " ";
                            _message += _list.at(2);
                        } catch (exception &e) {
                            cerr << "Encode: PROT_RESP: Malformed list for shutdown" << endl;
                            return false;
                        }
                        break;
                    }
                    case C_JOB_DONE: {
                        _message = "j";
                        _message += _list.at(1);
                        break;
                    }
                    default: {
                        return false;
                    }
                }
                } else {
                    cout << "Cannot encode a string. Use with the list constructor" << endl;
                    return false;
                }
                break;
            }

            case PROT_P_REQ: {
                switch(hf::get_p_com(_list)) {
                    case C_SUB: {
                        try {
                            _message = "0";
                            for (size_t i = 1; i < _list.size(); i++) {
                                _message += _list.at(i);
                                _message += " ";
                            }
                        } catch (exception& e) {
                            cout << "Encode: PROT_P_REQ: Malformed list for submit" << endl;
                            return false;
                        }
                        break;
                    }
                    case C_RES:
                        _message = "2";
                        try {
                            _message += _list.at(1);
                            break;
                        } catch (exception &e) {
                            cerr << "Encode: PROT_P_REQ: Malformed list for pool suspend" << endl;
                            return false;
                        }
                    case C_SUSP: {
                        _message = "1";
                        try {
                            _message += _list.at(1);
                            break;
                        } catch (exception &e) {
                            cerr << "Encode: PROT_P_REQ: Malformed list for pool suspend" << endl;
                            return false;
                        }
                    }
                    default: {
                        return false;
                    }
                }
                break;
            }

            case PROT_P_RESP: {
                _message = _list.at(0);
                try {
                    _message += _list.at(1);
                } catch (exception &e) {
                    cerr << "Encode: PROT_P_RESP: Malformed list for pool response" << endl;
                    return false;
                }
                break;
            }

            default: {
                return false;
            }
        }
        return true;
    }

    bool message::decode() {
        char op = _message[0];
        if (_using_list) {
            cout << "Cannot decode list. Use with the string constructor" << endl;
            return false;
        }

        switch (_mode) {

            // Request protocol
            case PROT_REQ: {
                switch (op) {
                    case '0': {
                        _list.push("0");
                        _list.push(_message.substr(1, ((int) _message.length() - 1)).split(' ').at(0));
                        my_vector<my_string> tmp_vec = _message.substr(1, ((int) _message.length() - 1)).split(' ');
                        for (size_t i = 1; i < tmp_vec.size(); i++) {
                            _list.push(tmp_vec.at(i));
                        }

                        break;
                    }
                    case '2': {
                      _list.push("2");
                      my_string tmp = _message.substr(1, (int) (_message.length() - 1));
                      if (tmp == "") {
                        _list.push("-1");
                      } else {
                      _list.push(tmp);
                      }

                      break;
                    }
                    case '1':
                    case '6':
                    case '7': {
                        _list.push(my_string(_message.c_str()[0] - 48));
                        char *tmp = new char[6];
                        for (size_t i = 1; i < 7; i++) {
                            tmp[i - 1] = _message.c_str()[i];
                            if (_message.c_str()[i] == '\0') break;
                        }
                        _list.push(my_string(tmp));
                        delete[] tmp;
                        break;
                    }
                    case '3':
                    case '4':
                    case '5':
                    case '8':
                        _list.push(_message);
                        break;
                    case 'j': {
                        _list.push("j");
                        //_list.push(_message.substr(1,
                        //          (int) (_message.length() - 1)));
                        my_string tmp =
                          _message.substr(1, (int) (_message.length() - 1));
                        my_vector<my_string> tmp_vec = tmp.split(' ');
                        try {
                          _list.push(tmp_vec.at(0));
                          _list.push(tmp_vec.at(1));
                        } catch (exception &e) {
                          cout << "Decode: PROT_REQ: Job Death: Malformed message" << endl;
                        }
                        break;
                    }
                    default:
                        return false;
                }
                break;
            }

            // Response protocol
            case PROT_RESP: {
                switch(op) {
                    case '0': {
                        _list.push("0");
                        my_vector<my_string> tmp_vec = _message.substr(1, ((int) _message.length() - 1))
                                .split(' ');

                        for (size_t i = 0; i < tmp_vec.size(); i++) {
                            _list.push(tmp_vec.at(i));
                        }
                        break;
                    }
                    case '1': {
                        _list.push("1");
                        if (_message[1] == '0') {
                            _list.push(my_string("0"));
                            try {
                                my_vector<my_string> tmp_vec = _message.split(' ');
                                _list.push(tmp_vec.at((int) tmp_vec.size() - 1));
                            } catch (exception &e) {
                                cerr << "Decode: PROT_RESP: Malformed message: " << _message << endl;
                                cerr << e.what() << endl;
                                return false;
                            }
                            break;
                        }

                        try {
                            _list.push(my_string(_message[1]));
                        } catch (exception &e) {
                            cerr << "Decode: PROT_RESP: Malformed message: " << _message << endl;
                            cerr << e.what() << endl;
                            return false;
                        }
                        break;
                    }
                    case '4': {
                        _list.push("4");
                        my_vector<my_string> tmp_sp = _message.substr(1,
                            ((int) _message.length() - 1))
                                .split(' ');

                        for (size_t i = 0; i < tmp_sp.size(); i++) {
                            _list.push(tmp_sp.at(i));
                        }
                        break;
                    }
                    case '2': {
                        _list.push("2");
                        if (_message.length() == 1) {
                          break;
                        }
                        my_vector<my_string> tmp_sp = _message.substr(1, ((int) _message.length() - 1))
                                .split(' ');

                        for (size_t i = 0; i < tmp_sp.size(); i++) {
                            _list.push(tmp_sp.at(i));
                        }
                        break;
                    }
                    case '3': {

                        _list.push("3");
                        my_vector<my_string> tmp_vec = _message.substr(1,
                            ((int) _message.length() - 1))
                                .split(' ');
                        for (size_t i = 0; i < tmp_vec.size(); i++) {
                            _list.push(tmp_vec.at(i));
                        }
                        break;
                    }
                    case '8': {
                        _list.push("8");
                        my_vector<my_string> tmp_vec = _message.substr(1,
                            ((int) _message.length() - 1))
                                .split(' ');

                        for (size_t i = 0; i < tmp_vec.size(); i++) {
                            _list.push(tmp_vec.at(i));
                        }

                        break;
                    }
                    case '5': {
                        _list.push("5");
                        my_vector<my_string> tmp_vec = _message.substr(1, ((int) _message.length() - 1))
                                .split(' ');
                        for (size_t i = 0; i < tmp_vec.size(); i++) {
                            _list.push(tmp_vec.at(i));
                        }
                        break;
                    }
                    case '6': {
                        _list.push("6");
                        try {
                            _list.push(_message[1]);
                        } catch (exception &e) {
                            cerr << "Decode: PROT_RESP: Malformed message: " << _message << endl;
                            return false;
                        }
                        break;
                    }
                    case '7': {
                        _list.push("7");
                        try {
                            _list.push(_message[1]);
                        } catch (exception &e) {
                            cerr << "Decode: PROT_RESP: Malformed message: " << _message << endl;
                            return false;
                        }
                        break;
                    }
                    case 'j': {
                        _list.push("j");
                        try {
                          _list.push(_message[1]);
                        } catch (exception &e) {
                          cout << "Decode: PROT_RESP: Malformed message for job death: " << _message << endl;
                        }
                        break;
                    }
                    default: {
                      return false;
                    }
                }
                break;
            }

            case PROT_P_REQ: {
                    switch(op) {
                        case '0': {
                            _list.push(my_string("0"));
                            my_vector<my_string> tmp_vec = _message.substr(1, ((int) _message.length() - 1))
                                    .split(' ');

                            for (size_t i = 0; i < tmp_vec.size(); i++) {
                                _list.push(tmp_vec.at(i));
                            }
                            break;
                        }
                        case '2':
                        case '1': {
                            _list.push(my_string(op));
                            try {
                                my_string tmp = _message.substr(1,
                                          (int) (_message.length() - 1));
                                _list.push(tmp);
                            } catch (exception &e) {
                                cerr << "Malformed message: " << _message << endl;
                                return false;
                            }
                            break;
                        }
                        default: {
                            return false;
                        }
                    }
                break;
            }

            case PROT_P_RESP: {
                _list.push(my_string(op));
                my_string tmp = _message.substr(1, (_message.length() - 1));
                _list.push(tmp);
                break;
            }

            default: {
                cout << "Unknown protocol: " << _mode << endl;
                return false;
            }
        }

        return true;
    }


    my_vector<my_string> message::get_list() { return _list; }

    my_string message::get_message() { return _message; }

}
