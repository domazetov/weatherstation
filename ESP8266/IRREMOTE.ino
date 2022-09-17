#if ENABLE_IR
boolean receive_ir_signal(decode_results* results){
    boolean ret_val = false;

    if (irrecv.decode(results)){
        LOG_SERIAL.println("Captured!");
        save_to_eeprom();
        delay(1000);
        irrecv.resume();
        LOG_SERIAL.println(resultToHumanReadableBasic(results));
        ret_val = true;
    }else{
        LOG_SERIAL.print(".");
        digitalWrite(LED_PIN, LOW);
        delay(500);
        digitalWrite(LED_PIN, HIGH);
        delay(500);
    }
    return ret_val;
}

boolean send_ir_signal(decode_results* results){
    boolean ret_val = false;
    LOG_SERIAL.println(resultToHumanReadableBasic(results));
    decode_type_t ac_protocol = results->decode_type;
    uint16_t ac_size = results->bits;

    if (hasACState(ac_protocol)){
        ret_val = irsend.send(ac_protocol, results->state, ac_size / 8);
    }else{
        ret_val = irsend.send(ac_protocol, results->value, ac_size);
    }

    yield();

    if(ret_val){
        LOG_SERIAL.println("AC Signal sent");
    }
    return ret_val;
}
#endif